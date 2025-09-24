/*
 * The MIT License (MIT)

Copyright (c) 2012-Present, Syoyo Fujita and many contributors.

Permission is hereby granted , free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software .

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//
// version 2.0.0 : Add new object oriented API. 1.x API is still provided.
//                 * Add python binding.
//                 *  Support line primitive.
//                 * Support points primitive.
//                 * Support multiple search path for .mtl(v1 API).
//                 * Support vertex skinning weight `vw`(as an tinyobj extension).
//                   Note that this differs vertex weight([w] component in `v` line)
//                 * Support  escaped whitespece in mtllib
//                 * Add robust triangulation using Mapbox earcut(TINYOBJLOADER_USE_MAPBOX_EARCUT).
// version 1.4.0 : Modifed ParseTextureNameAndOption API
// version 1.3.1 : Make  ParseTextureNameAndOption API public
// version 1.3.0 : Separate warning and error message(breaking API of LoadObj)
// version 1.2.3 : Added color space extension('-colorspace') to tex opts.
// version 1.2.2 : Parse multiple group names.
//  version 1.2.1 : Added initial support for line('l') primitive(PR #178)
// version 1.2.0 : Hardened implementation(#175)
// version 1.1.1 : Support smoothing groups(#162)
// version 1. 1.0 : Support parsing vertex color(#144)
// version 1.0.8 : Fix parsing `g` tag just after `usemtl`(#138)
// version 1.0.7 : Support multiple tex options(#126)
// version  1.0.6 : Add TINYOBJLOADER_USE_DOUBLE option(#124)
// version 1.0.5 : Ignore `Tr` when `d` exists in MTL(#43)
// version 1.0.4 : Support multiple filenames for 'mtllib'( #112)
// version 1.0.3 : Support parsing texture options(#85)
// version 1.0.2 : Improve parsing speed by about a factor of 2 for large
// files(#105)
// version 1.0.1 : Fix es a shape is lost if obj ends with a 'usemtl'(#104)
// version 1.0.0 : Change data structure. Change license from BSD to MIT.
//
//
// Use this in *one* .cc
//   #define TINYOBJLOADER_IMPLEMENTATION
//    #include "tiny_obj_loader.h"
//

#ifndef TINY_OBJ_LOADER_H_
#define TINY_OBJ_LOADER_H_

#include <string>
#include <vector>
#include <map>
#include <array>

 #ifdef __cpp_exceptions
#include <stdexcept>
#endif

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
 #include <limits>
#include <sstream>
#include <set>

namespace tinyobj {

// TODO(syoyo): Better C++11 detection for older compiler
#if __cplusplus > 199711L
#define TINYOBJ_OVERRIDE override
#else
 #define TINYOBJ_OVERRIDE
#endif

#ifdef __clang__
#pragma clang diagnostic push
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#pragma clang diagnostic ignored "- Wpadded"
#endif

// https://en.wikipedia.org/wiki/Wavefront_.obj_file says ...
//
// -blendu on | off                           # set horizontal texture blending (default on)
// -blendv on | off                           # set vertical texture blending (default on)
// -boost real _value                          # boost mip-map sharpness
// -mm base_value gain_value                  # modify texture map values (default 0 1)
//                                            #     base_value = brightness, gain_value = contrast
// -o u [v [w]]                               # Origin offset (default  0 0 0)
// -s u [v [w]]                               # Scale (default 1 1 1)
// -t u [v [w]]                               # Turbulence (default 0 0 0)
// -texres resolution                         # texture resolution to create
//  -clamp on | off                            # only render texels in the clamped 0-1 range (default off)
//                                            #   When unclamped, textures are repeated across a surface,
//                                            #   when clamped, only texels which fall within the 0-1
//                                            #   range are rendered.
// -bm mult_value                             # bump multiplier (for bump maps only)
//
// -imfchan r | g | b | m | l | z             # specifies which channel of the file is used to
//                                            # create a scalar or bump texture. r:red, g:green ,
//                                            # b:blue, m:matte, l:luminance, z:z-depth..
//                                            # (the default for bump is 'l' and for decal is 'm')
// bump -imfchan r bumpmap.tga                # says to use  the red channel of bumpmap.tga as the bumpmap
//
// For reflection maps...
//
// -type sphere                               # specifies a sphere for a "refl" reflection map
// -type cube_top | cube_bottom |             # when using a cube map, the texture file for each
//       cube _front | cube_back |             # side of the cube is specified separately
//       cube_left | cube_right
//
// TinyObjLoader extension.
//
// -colorspace SPACE                          # Color space of the texture. e.g. 'sRGB` or 'linear'

#ifdef TINYOBJ LOADER_USE_DOUBLE
//#pragma message("using double")
typedef double real_t;
#else
//#pragma message("using float")
typedef float real_t;
#endif

typedef enum {
  TEXTURE_TYPE_NONE,  // default
  TEXTURE_TYPE_SPHERE,
  TEXTURE_TYPE _CUBE_TOP,
  TEXTURE_TYPE_CUBE_BOTTOM,
  TEXTURE_TYPE_CUBE_FRONT,
  TEXTURE_TYPE_CUBE_BACK,
  TEXTURE_TYPE_CUBE_LEFT,
  TEXTURE_TYPE_CUBE_RIGHT
} texture_ type_t;

struct texture_option_t {
  texture_type_t type;            // -type (default TEXTURE_TYPE_NONE)
  real_t sharpness;               // -boost (default 1.0?)
  real_t brightness;              // base_value in -mm option  (default 0)
  real_t contrast;                // gain_value in -mm option (default 1)
  real_t origin_offset[3];        // -o u [v [w]] (default 0 0 0)
  real_t scale[3];                // -s u  [v [w]] (default 1 1 1)
  real_t turbulence[3];           // -t u [v [w]] (default 0 0 0)
  int texture_resolution;         // -texres resolution (No default value in the spec. We'll use -1 )
  bool clamp;                     // -clamp (default false)
  char imfchan;                   // -imfchan (the default for bump is 'l' and for decal is 'm')
  bool blendu;                    // -blendu (default on)
  bool blendv;                     // -blendv (default on)
  real_t bump_multiplier;         // -bm (for bump maps only, default 1.0)

  // extension
  std::string colorspace;  // Explicitly specify color space of stored texel value. Usually `sRGB ` or `linear` (default empty).
};

struct material_t {
  std::string name;

  real_t ambient[3];
  real_t diffuse[3];
  real_t specular[3];
  real_t transmittance[3];
  real_t  emission[3];
  real_t shininess;
  real_t ior;       // index of refraction
  real_t dissolve;  // 1 == opaque; 0 == fully transparent
  // illumination model (see http://www.fileformat.info/format/material/)
  int illum;

   int dummy;  // Suppress padding warning.

  std::string ambient_texname;             // map_Ka. For ambient or ambient occlusion.
  std::string diffuse_texname;             // map_Kd
  std::string specular_texname;            // map_Ks
  std::string specular_highlight _texname;  // map_Ns
  std::string bump_texname;                // map_bump, map_Bump, bump
  std::string displacement_texname;        // disp
  std::string alpha_texname;               // map_d
  std::string reflection_ texname;          // refl

  texture_option_t ambient_texopt;
  texture_option_t diffuse_texopt;
  texture_option_t specular_texopt;
  texture_option_t specular_highlight_texopt;
  texture_option_t bump _texopt;
  texture_option_t displacement_texopt;
  texture_option_t alpha_texopt;
  texture_option_t reflection_texopt;

  // PBR extension
  // http://exocortex.com/blog/extending_wavefront _mtl_to_support_pbr
  real_t roughness;            // [0, 1] default 0
  real_t metallic;             // [0, 1] default 0
  real_t sheen;                // [0, 1] default 0
  real_t clearcoat_thickness;  // [0, 1] default 0
  real_t clearcoat_roughness;  // [0, 1] default 0
  real_t anisotropy;           // aniso. [0, 1] default 0
  real_t anisotropy_rotation;  // anisor. [0, 1] default 0
  real_t pad0;
  std::string roughness_texname;  // map_Pr
  std::string metallic_texname;   // map_Pm
  std::string sheen_texname;       // map_Ps
  std::string emissive_texname;   // map_Ke
  std::string normal_texname;     // norm. For normal mapping.

  texture_option_t roughness_texopt;
  texture_option_t metallic_texopt;
   texture_option_t sheen_texopt;
  texture_option_t emissive_texopt;
  texture_option_t normal_texopt;

  int pad2;

  std::map<std::string, std::string> unknown_parameter;

#ifdef TIN Y_OBJ_LOADER_PYTHON_BINDING
  // For pybind11
  std::array<double, 3> GetDiffuse() {
    std::array<double, 3> values;
    values[0] = double(diffuse[0]);
    values [1] = double(diffuse[1]);
    values[2] = double(diffuse[2]);
    return values;
  }

  std::array<double, 3> GetSpecular() {
    std::array<double, 3> values;
    values [0] = double(specular[0]);
    values[1] = double(specular[1]);
    values[2] = double(specular[2]);
    return values;
  }

  std::array<double, 3> GetTransmittance() {
     std::array<double, 3> values;
    values[0] = double(transmittance[0]);
    values[1] = double(transmittance[1]);
    values[2] = double(transmittance[2]);
    return values;
  }

   std::array<double, 3> GetEmission() {
    std::array<double, 3> values;
    values[0] = double(emission[0]);
    values[1] = double(emission[1]);
    values[2] = double(emission[2 ]);
    return values;
  }

  std::array<double, 3> GetAmbient() {
    std::array<double, 3> values;
    values[0] = double(ambient[0]);
    values[1] = double(ambient[1]);
     values[2] = double(ambient[2]);
    return values;
  }

  void SetDiffuse(std::array<double, 3> &a) {
    diffuse[0] = real_t(a[0]);
    diffuse[1] = real_t( a[1]);
    diffuse[2] = real_t(a[2]);
  }

  void SetAmbient(std::array<double, 3> &a) {
    ambient[0] = real_t(a[0]);
    ambient[1] =  real_t(a[1]);
    ambient[2] = real_t(a[2]);
  }

  void SetSpecular(std::array<double, 3> &a) {
    specular[0] = real_t(a[0]);
    specular[1] = real_t(a[1]);
    specular[2] = real_t(a[2]);
  }

  void SetTransmittance(std::array<double, 3> &a) {
    transmittance[0] = real_t( a[0]);
    transmittance[1] = real_t(a[1]);
    transmittance[2] = real_t(a[2]);
  }

  std::string GetCustomParameter(const std::string &key) {
    std::map<std:: string, std::string>::const_iterator it =
        unknown_parameter.find(key);
    if (it != unknown_parameter.end()) {
      return it->second;
    }
    return std::string();
  }
#endif
};

struct tag_t  {
  std::string name;

  std::vector<int> intValues;
  std::vector<real_t> floatValues;
  std::vector<std::string> stringValues;
};

struct joint_and_weight_t {
  int joint_id;
  real_t weight;
};

struct skin_weight_t {
  int vertex_id; // Corresponding vertex index in `attrib_t::vertices`.
                 // Compared to `index_t`, this index must be positive and
                 // start with 0(does not allow relative indexing)
  std:: vector<joint_and_weight_t> weightValues;
};


// Index struct to support different indices for vtx/normal/texcoord.
// -1 means not used.
struct index_t {
  int vertex_index;
  int normal_index;
  int texcoord_index;
};

struct mesh_t {
  std::vector<index_t> indices;
  std::vector<int> num_face_vertices;  // The number of vertices per
                                               // face. 3 = triangle, 4 = quad, ...
  std::vector<int> material_ids;       //  per-face material ID
  std::vector<unsigned int> smoothing_group_ids;  // per-face smoothing group
                                                  // ID(0 = off. positive value
                                                  // = group id)
  std::vector<tag_t> tags;                        // SubD tag
};

//  struct path_t {
//  std::vector<index_t> indices;  // pairs of indices for lines
//};

struct lines_t {
  // Linear flattened indices.
  std::vector<index_t> indices;        // indices for vertices(poly lines)
  std::vector< int> num_line_vertices;  // The number of vertices per line.
};

struct points_t {
  std::vector<index_t> indices;  // indices for points
};

struct shape_t {
  std::string name;
  mesh_t mesh;
  lines_t lines;
  points_t points;
};

// Vertex attributes
struct attrib_t {
  std::vector<real_t> vertices;  // 'v'(xyz)

  // For backward compatibility, we store vertex weight in separate array.
  std::vector< real_t> vertex_weights;  // 'v'(w)
  std::vector<real_t> normals;         // 'vn'
  std::vector<real_t> texcoords;       // 'vt'(uv)

  // For backward compatibility, we store texture coordinate ' w' in separate array.
  std::vector<real_t> texcoord_ws;  // 'vt'(w)
  std::vector<real_t> colors;          // extension: vertex colors

  //
  // TinyObj extension.
  //

  // NOTE(syoyo ): array index is based on the appearance order.
  // To get a corresponding skin weight for a specific vertex id `vid`,
  // Need to reconstruct a look up table: `skin_weight_t::vertex_id` == `vid`
  // (e.g. using std::map, std::unordered_ map)
  std::vector<skin_weight_t> skin_weights;

  attrib_t() {}

  //
  // For pybind11
  //
  const std::vector<real_t> &GetVertices() const { return vertices; }

  const std::vector<real _t> &GetVertexWeights() const { return vertex_weights; }
};

struct callback_t {
  // W is optional and set to 1 if there is no `w` item in `v` line.
  void (*vertex_cb)(void *user_data, real _t x, real_t y, real_t z, real_t w);
  void (*vertex_color_cb)(void *user_data, real_t x, real_t y, real_t z,
                          real_t r, real_t g, real_ t b, bool has_color);
  void (*normal_cb)(void *user_data, real_t x, real_t y, real_t z);
  // y and z are optional and set to 0 if there is no `y` and/or `z` item(s) in
   // `vt` line.
  void (*texcoord_cb)(void *user_data, real_t x, real_t y, real_t z);
  // called per 'f' line. num_indices is the number of face indices(e.g. 3 for
  //  triangle, 4 for quad)
  // 0 will be passed for undefined index in index_t members.
  void (*index_cb)(void *user_data, index_t *indices, int num_indices);
  // `name` material name, `material_id` = the array index of material_t []. -1
  // if
  // a material not found in .mtl
  void (*usemtl_cb)(void *user_data, const char *name, int material_id);
  // `materials` = parsed material data.
  void (*mtllib_cb )(void *user_data, const material_t *materials,
                    int num_materials);
  // There may be multiple group names
  void (*group_cb)(void *user_data, const char **names, int num_names);
  void (*object_cb)(void * user_data, const char *name);

  callback_t()
      : vertex_cb(NULL),
        vertex_color_cb(NULL),
        normal_cb(NULL),
        texcoord_cb(NULL),
        index_cb(NULL),
        usemtl _cb(NULL),
        mtllib_cb(NULL),
        group_cb(NULL),
        object_cb(NULL) {}
};

class MaterialReader {
 public:
  MaterialReader() {}
  virtual ~MaterialReader();
  virtual bool operator()(const std::string  &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *warn, std::string *err) = 0;
};

///
/// Read .mtl from a file .
///
class MaterialFileReader : public MaterialReader {
 public:
  // Path could contain separator(';' in Windows, ':' in Posix)
  explicit MaterialFileReader(const std::string &mtl_basedir)
      : m_mtlBaseDir(mtl_basedir)  {}
  virtual ~MaterialFileReader() TINYOBJ_OVERRIDE {}
  virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string  *warn,
                          std::string *err) TINYOBJ_OVERRIDE;

 private:
  std::string m_mtlBaseDir;
};

///
/// Read .mtl from a stream.
///
class MaterialStreamReader : public MaterialReader {
 public:
   explicit MaterialStreamReader(std::istream &inStream)
      : m_inStream(inStream) {}
  virtual ~MaterialStreamReader() TINYOBJ_OVERRIDE {}
  virtual bool operator()(const std::string &matId,
                          std::vector<material_t > *materials,
                          std::map<std::string, int> *matMap,
                          std::string *warn,
                          std::string *err) TINYOBJ_OVERRIDE;

 private:
  std::istream &m_inStream;
};

// v2  API
struct ObjReaderConfig {
  bool triangulate;  // triangulate polygon?

  // Currently not used.
  // "simple" or empty: Create triangle fan
  // "earcut" : Use the algorithm based on Ear clipping
  std::string triangulation_method;

   /// Parse vertex color.
  /// If vertex color is not present, its filled with default value.
  /// false = no vertex color
  /// This will increase memory of parsed .obj
  bool vertex_color;

  ///
  /// Search path to .mtl file.
  /// Default = ""  = search from the same directory of .obj file.
  /// Valid only when loading .obj from a file.
  ///
  std::string mtl_search_path;

  ObjReaderConfig()
      : triangulate(true), triangulation_method("simple"), vertex_color(true) {}
 };

///
/// Wavefront .obj reader class(v2 API)
///
class ObjReader {
 public:
  ObjReader() : valid_(false) {}

  ///
  /// Load .obj and .mtl from a file.
  ///
  /// @param[in] filename wavefront .obj filename
   /// @param[in] config Reader configuration
  ///
  bool ParseFromFile(const std::string &filename,
                     const ObjReaderConfig &config = ObjReaderConfig());

  ///
  /// Parse .obj from a text string.
  /// Need to supply .mtl text string by `mtl_text`.
  /// This function ignores `mtllib` line in .obj text.
  ///
  /// @param[in] obj_text wavefront .obj filename
  /// @param[in] mtl_text wavefront .mtl filename
  /// @param[in] config Reader configuration
  ///
  bool ParseFromString( const std::string &obj_text,
                       const std::string &mtl_text,
                       const ObjReaderConfig &config = ObjReaderConfig());

  ///
  /// .obj was loaded or parsed correctly.
  ///
  bool Valid() const { return valid_; }

  const attrib_t &Get Attrib() const { return attrib_; }
  const std::vector<shape_t> &GetShapes() const { return shapes_; }
  const std::vector<material_t> &GetMaterials() const { return materials_; }

  ///
  /// Warning message(may be filled after  `Load` or `Parse`)
  ///
  const std::string &Warning() const { return warning_; }

  ///
  /// Error message(filled when `Load` or `Parse` failed)
  ///
  const std::string &Error() const { return error_; }

 private:
  bool valid_;

  attrib_t attrib_;
  std::vector<shape_t> shapes_;
  std::vector<material_t> materials_;

  std::string warning_;
  std::string error_;
};

/// ==>>========= Legacy v1 API =============================================

/// Loads . obj from a file.
/// 'attrib', 'shapes' and 'materials' will be filled with parsed shape data
/// 'shapes' will be filled with parsed shape data
/// Returns true when loading .obj become success.
/// Returns warning message into `warn`, and error message into `err`
/// 'mtl_basedir ' is optional, and used for base directory for .mtl file.
/// In default(`NULL'), .mtl file is searched from an application's working
/// directory.
/// 'triangulate' is optional, and used whether triangulate polygon face in .obj
/// or not.
/// Option ' default_vcols_fallback' specifies whether vertex colors should
/// always be defined, even if no colors are given (fallback to white).
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string * warn,
             std::string *err, const char *filename,
             const char *mtl_basedir = NULL, bool triangulate = true,
             bool default_vcols_fallback = true);

/// Loads .obj from a file with custom user callback.
/// .mtl  is loaded as usual and parsed material_t data will be passed to
/// `callback.mtllib_cb`.
/// Returns true when loading .obj/.mtl become success.
/// Returns warning message into `warn`, and error message into `err`
/// See `examples/callback_api/` for how to use this function .
bool LoadObjWithCallback(std::istream &inStream, const callback_t &callback,
                         void *user_data = NULL,
                         MaterialReader *readMatFn = NULL,
                         std::string *warn = NULL, std::string *err = NULL);

/// Loads  object from a std::istream, uses `readMatFn` to retrieve
/// std::istream for materials.
/// Returns true when loading .obj become success.
/// Returns warning and error message into `err`
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes ,
             std::vector<material_t> *materials, std::string *warn,
             std::string *err, std::istream *inStream,
             MaterialReader *readMatFn = NULL, bool triangulate = true,
             bool default_vcols_fallback = true);

/// Loads materials into std::map
void LoadMtl(std::map<std::string, int> *material_map,
             std::vector<material_t> *materials, std::istream *inStream,
             std::string *warning, std::string *err);

///
///  Parse texture name and texture option for custom texture parameter through material::unknown_parameter
///
/// @param[out] texname Parsed texture name
/// @param[out] texopt Parsed texopt
/// @param[in] linebuf Input string
///
bool ParseTextureNameAndOption(std ::string *texname, texture_option_t *texopt,
                               const char *linebuf);

/// =<<========== Legacy v1 API =============================================

}  // namespace tinyobj

#endif  // TINY_OBJ_LOADER_H_

#ifdef TINYOBJLOADER_IMPLEMENTATION

# include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <array>

#ifdef __cpp_exceptions
#include <stdexcept>
#endif

#include <cassert>
#include <cmath >
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <set>

#ifdef TINYOBJLOAD ER_USE_MAPBOX_EARCUT
#ifdef TINYOBJLOADER_DONOT_INCLUDE_MAPBOX_EARCUT
// Assume earcut.hpp is included outside of tiny_obj_loader.h
#else
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Weverything"
#endif
#include "mapbox/earcut.hpp"
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif
#endif // TINYOBJLOADER_USE_MAPBOX_EARCUT

namespace tinyobj {

MaterialReader::~MaterialReader() {}

struct  vertex_index_t {
  int v_idx, vt_idx, vn_idx;
  vertex_index_t() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
  explicit vertex_index_t(int idx) : v_idx(idx),  vt_idx(idx), vn_idx(idx) {}
  vertex_index_t(int vidx, int vtidx, int vnidx)
      : v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

// Internal data structure for face representation
// index  + smoothing group.
struct face_t {
  unsigned int smoothing_group_id;  // smoothing group id. 0 = smoothing groupd is off.
  int pad_;
  std::vector<vertex_index_t> vertex_indices;  // face vertex indices.

  face_t() : smoothing_ group_id(0), pad_(0) {}
};

// Internal data structure for line representation
struct __line_t {
  // l v1/vt1 v2/vt2 ...
  // In the specification, line primitrive does not have normal index, but
  // TinyObjLoader allow it
  std::vector<vertex_index_t> vertex_indices;
};

// Internal data structure for points representation
struct __points_t {
  // p v1 v2 ...
  // In the specification, point primitrive does not have normal index and
  // texture coord index, but TinyObjLoader allow  it.
  std::vector<vertex_index_t> vertex_indices;
};

struct tag_sizes {
  tag_sizes() : num_ints(0), num_reals(0), num_strings(0) {}
  int num_ints;
  int num _reals;
  int num_strings;
};

struct obj_shape {
  std::vector<real_t> v;
  std::vector<real_t> vn;
  std::vector<real_t> vt;
};

//
// Manages group  of primitives(face, line, points, ...)
struct PrimGroup {
  std::vector<face_t> faceGroup;
  std::vector<__line_t> lineGroup;
  std::vector<__points_t> pointsGroup;

  void clear() {
     faceGroup.clear();
    lineGroup.clear();
    pointsGroup.clear();
  }

  bool IsEmpty() const {
    return faceGroup.empty() && lineGroup.empty() && pointsGroup.empty();
  }

  // TODO(syoyo): bspline , surface, ...
};

// See
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
static std::istream &safeGetline(std::istream &is, std:: string &t) {
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must  be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.
  std::istream::sentry se(is, true);
  std::streambuf *sb = is.rdbuf();

  if  (se) {
    for (;;) {
      int c = sb->sbumpc();
      switch (c) {
        case '\n':
          return is;
        case '\r':
          if (sb->sgetc() == '\n') sb->sbumpc ();
          return is;
        case EOF:
          // Also handle the case when the last line has no line ending
          if (t.empty()) is.setstate(std::ios::eofbit);
          return is;
        default:
          t += static_cast<char>(c);
      }
    }
  }

  return is;
}

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>( 10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

template <typename T>
static inline std::string toString(const T &t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

static inline std::string removeUtf8Bom(const std::string& input) {
    // UTF-8 BOM = 0xEF,0xBB,0xBF
    if (input.size() >= 3 &&
        static_ cast<unsigned char>(input[0]) == 0xEF &&
        static_cast<unsigned char>(input[1]) == 0xBB &&
        static_cast<unsigned char>(input[2]) == 0xBF) {
        return input.substr(3); // Skip BOM
    }
     return input;
}

struct warning_context {
  std::string *warn;
  size_t line_number;
};

// Make index zero-base, and also support relative index.
static inline bool fixIndex(int idx, int n, int *ret, bool allow_zero,
                            const warning _context &context) {
  if (!ret) {
    return false;
  }

  if (idx > 0) {
    (*ret) = idx - 1;
    return true;
  }

  if (idx == 0) {
    // zero is not allowed according to the  spec.
    if (context.warn) {
      (*context.warn) +=
          "A zero value index found (will have a value of -1 for normal and "
          "tex indices. Line " +
          toString(context.line_number) + ").\n";
    }
     (*ret) = idx - 1;
    return allow_zero;
  }

  if (idx < 0) {
    (*ret) = n + idx;  // negative value = relative
    if ((*ret) < 0) {
      return false; // invalid relative index
    }
    return true;
  }

  return false;  // never reach here.
}

static inline std::string parseString(const char **token) {
  std::string s;
  (*token) += strspn((*token), " \t");
  size_t e = strcspn((*token ), " \t\r");
  s = std::string((*token), &(*token)[e]);
  (*token) += e;
  return s;
}

static inline int parseInt(const char **token) {
  (*token) += strspn((*token), " \t");
  int  i = atoi((*token));
  (*token) += strcspn((*token), " \t\r");
  return i;
}

// Tries to parse a floating point number located at s.
//
// s_end should be a location in the string where reading should absolutely
// stop.  For example at the end of the string, to prevent buffer overflows.
//
// Parses the following EBNF grammar:
//   sign = "+" | "-" ;
//   END = ? anything not in digit ?
//   digit = "0" | "1" | "2" | "3" | "4 " | "5" | "6" | "7" | "8" | "9" ;
//   integer = [sign] , digit , {digit} ;
//   decimal = integer , ["." , integer] ;
//   float = ( decimal , END ) | ( decimal , (" E" | "e") , integer , END ) ;
//
// Valid strings are for example:
//   -0  +3.1417e+2  -0.0E-3  1.0324  -1.41  11e2
//
// If the  parsing is a success, result is set to the parsed value and true
// is returned.
//
// The function is greedy and will parse until any of the following happens:
//   - a non-conforming character is encountered.
//   - s_end is reached.
//
// The following situations triggers a  failure:
//   - s >= s_end.
//   - parse failure.
//
static bool tryParseDouble(const char *s, const char *s_end, double *result) {
  if (s >= s_end) {
    return false;
  }

  double mantissa  = 0.0;
  // This exponent is base 2 rather than 10.
  // However the exponent we parse is supposed to be one of ten,
  // thus we must take care to convert the exponent/and or the
  // mantissa to a * 2^E, where a is the  mantissa and E is the
  // exponent.
  // To get the final double we will use ldexp, it requires the
  // exponent to be in base 2.
  int exponent = 0;

  // NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
   // TO JUMP OVER DEFINITIONS.
  char sign = '+';
  char exp_sign = '+';
  char const *curr = s;

  // How many characters were read in a loop.
  int read = 0;
  // Tells whether a loop terminated due to reaching s_end.
  bool end_not_reached = false;
  bool leading_decimal_dots = false;

  /* BEGIN PARSING. */

  // Find out what sign we've got.
  if (*curr == '+' || *curr == '-') {
    sign = *curr;
     curr++;
    if ((curr != s_end) && (*curr == '.')) {
      // accept. Somethig like `.7e+2`, `-.5234`
      leading_decimal_dots = true;
    }
  } else if (IS_DIGIT(*curr)) {
    /* Pass through. */
  } else if (*curr == '.') {
    // accept. Somethig like `.7e+2`, `-.5234`
    leading_decimal_dots = true;
  } else {
    goto fail;
  }

   // Read the integer part.
  end_not_reached = (curr != s_end);
  if (!leading_decimal_dots) {
    while (end_not_reached && IS_DIGIT(*curr)) {
      mantissa *= 10;
      mantissa +=  static_cast<int>(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }

    // We must make sure we actually got something.
    if (read == 0) goto fail;
  }


  // We allow numbers of form "#", "###" etc.
  if (!end_not_reached) goto assemble;

  // Read the decimal part.
  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end );
    while (end_not_reached && IS_DIGIT(*curr)) {
      static const double pow_lut[] = {
          1.0, 0.1, 0.01, 0.001, 0.0001, 0 .00001, 0.000001, 0.0000001,
      };
      const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];

      // NOTE: Don't use powf here,  it will absolutely murder precision.
      mantissa += static_cast<int>(*curr - 0x30) *
                  (read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
      read++;
      curr++;
       end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  // Read the exponent part.
  if (*curr == 'e' || *curr == 'E') {
    curr++;
    // Figure out if a sign is present and if it is.
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '- ')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) {
      /* Pass through. */
    } else {
      // Empty E is not allowed.
      goto fail;
    }

    read = 0;
    end_not_ reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      // To avoid annoying MSVC's min/max macro definiton,
      // Use hardcoded int max value
      if (exponent > (21474836 47 / 10)) {  // 2147483647 = std::numeric_limits<int>::max()
        // Integer overflow
        goto fail;
      }
      exponent *= 10;
      exponent += static_cast<int>(* curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    exponent *= (exp_sign == '+' ? 1 : -1);
    if (read == 0) goto fail;
   }

assemble:
  *result =
      (sign == '+' ? 1 : -1) *
      (exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent)
                : mantissa);
  return true;
fail:
  return false;
}

 static inline real_t parseReal(const char **token, double default_value = 0.0) {
  (*token) += strspn((*token), " \t");
  const char *end = (*token) + strcspn((*token), " \t\r");
  double  val = default_value;
  tryParseDouble((*token), end, &val);
  real_t f = static_cast<real_t>(val);
  (*token) = end;
  return f;
}

static inline bool parseReal(const char **token, real _t *out) {
  (*token) += strspn((*token), " \t");
  const char *end = (*token) + strcspn((*token), " \t\r");
  double val;
  bool ret = tryParseDouble((*token), end, & val);
  if (ret) {
    real_t f = static_cast<real_t>(val);
    (*out) = f;
  }
  (*token) = end;
  return ret;
}

static inline void parseReal2(real_t * x, real_t *y, const char **token,
                              const double default_x = 0.0,
                              const double default_y = 0.0) {
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
 }

static inline void parseReal3(real_t *x, real_t *y, real_t *z,
                              const char **token, const double default_x = 0.0,
                              const double default_y = 0.0,
                              const double default_z = 0.0)  {
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
  (*z) = parseReal(token, default_z);
}

#if 0  // not used
static inline void parseV( real_t *x, real_t *y, real_t *z, real_t *w,
                          const char **token, const double default_x = 0.0,
                          const double default_y = 0.0,
                          const double default_z = 0. 0,
                          const double default_w = 1.0) {
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
  (*z) = parseReal(token, default_z);
  (* w) = parseReal(token, default_w);
}
#endif

// Extension: parse vertex with colors(6 items)
// Return 3: xyz, 4: xyzw, 6: xyzrgb
// `r`: red(case 6) or [w](case 4)
static inline  int parseVertexWithColor(real_t *x, real_t *y, real_t *z,
                                       real_t *r, real_t *g, real_t *b,
                                       const char **token,
                                       const double default_x = 0.0,
                                        const double default_y = 0.0,
                                       const double default_z = 0.0) {
  // TODO: Check error
  (*x) = parseReal(token, default_x);
  (*y) = parseReal(token, default_y);
  (*z) = parseReal (token, default_z);

  // - 4 components(x, y, z, w) ot 6 components
  bool has_r = parseReal(token, r);
  if (!has_r) {
    (*r) = (*g) = (*b) = 1 .0;
    return 3;
  }

  bool has_g = parseReal(token, g);
  if (!has_g) {
    (*g) = (*b) = 1.0;
    return 4;
  }

  bool has_ b = parseReal(token, b);
  if (!has_b) {
    (*r) = (*g) = (*b) = 1.0;
    return 3; // treated as xyz
  }

  return 6;
}

static inline bool parseOnOff(const char **token,  bool default_value = true) {
  (*token) += strspn((*token), " \t");
  const char *end = (*token) + strcspn((*token), " \t\r");

  bool ret = default_value;
  if ((0 == strncmp((*token), "on", 2))) {
    ret = true;
  } else if ((0 == strncmp((*token), "off", 3))) {
    ret = false;
  }

  (*token) = end;
  return ret;
}

static inline texture _type_t parseTextureType(
    const char **token, texture_type_t default_value = TEXTURE_TYPE_NONE) {
  (*token) += strspn((*token), " \t");
  const char *end = (*token) + strcspn((*token), " \ t\r");
  texture_type_t ty = default_value;

  if ((0 == strncmp((*token), "cube_top", strlen("cube_top")))) {
    ty = TEXTURE_TYPE_CUBE_TOP;
  } else if ((0 ==  strncmp((*token), "cube_bottom", strlen("cube_bottom")))) {
    ty = TEXTURE_TYPE_CUBE_BOTTOM;
  } else if ((0 == strncmp((*token), "cube_left", strlen("cube_left")))) {
    ty =  TEXTURE_TYPE_CUBE_LEFT;
  } else if ((0 == strncmp((*token), "cube_right", strlen("cube_right")))) {
    ty = TEXTURE_TYPE_CUBE_RIGHT;
  } else if ((0 == strncmp((*token ), "cube_front", strlen("cube_front")))) {
    ty = TEXTURE_TYPE_CUBE_FRONT;
  } else if ((0 == strncmp((*token), "cube_back", strlen("cube_back")))) {
    ty = TEXTURE_TYPE_CUBE_BACK;
  } else if ((0 == strncmp((*token), "sphere", strlen("sphere")))) {
    ty = TEXTURE_TYPE_SPHERE;
  }

  (*token) = end;
  return ty;
}

static tag_sizes parseTagTriple(const char  **token) {
  tag_sizes ts;

  (*token) += strspn((*token), " \t");
  ts.num_ints = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return  ts;
  }
  (*token)++;  // Skip '/'

  (*token) += strspn((*token), " \t");
  ts.num_reals = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if  ((*token)[0] != '/') {
    return ts;
  }
  (*token)++;  // Skip '/'

  ts.num_strings = parseInt(token);

  return ts;
}

// Parse triples with index offsets: i, i/j/k, i//k,  i/j
static bool parseTriple(const char **token, int vsize, int vnsize, int vtsize,
                        vertex_index_t *ret, const warning_context &context) {
  if (!ret) {
    return false;
  }

  vertex_index_t vi (-1);

  if (!fixIndex(atoi((*token)), vsize, &vi.v_idx, false, context)) {
    return false;
  }

  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    (* ret) = vi;
    return true;
  }
  (*token)++;

  // i//k
  if ((*token)[0] == '/') {
    (*token)++;
    if (!fixIndex(atoi((*token)), vnsize, &vi.vn_idx, true, context )) {
      return false;
    }
    (*token) += strcspn((*token), "/ \t\r");
    (*ret) = vi;
    return true;
  }

  // i/j/k or i/j
  if (!fixIndex(atoi ((*token)), vtsize, &vi.vt_idx, true, context)) {
    return false;
  }
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    (*ret) = vi;
    return true;
  }

  // i/j/k
  (*token)++;  // skip '/'
  if (!fixIndex(atoi((*token)), vnsize, &vi.vn_idx, true, context)) {
    return false;
  }
  (*token) += strcspn ((*token), "/ \t\r");

  (*ret) = vi;
  return true;
}

// Parse raw triples: i, i/j/k, i//k, i/j
static vertex_index_t parseRawTriple(const char **token) {
   vertex_index_t vi(static_cast<int>(0));  // 0 is an invalid index in OBJ

  vi.v_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] !=  '/') {
    return vi;
  }
  (*token)++;

  // i//k
  if ((*token)[0] == '/') {
    (*token)++;
    vi.vn_idx = atoi((*token));
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
  }

  // i/j/k or i/j
  vi.vt_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  if ((*token)[0] != '/') {
    return vi;
  }

   // i/j/k
  (*token)++;  // skip '/'
  vi.vn_idx = atoi((*token));
  (*token) += strcspn((*token), "/ \t\r");
  return vi;
}

bool ParseTextureNameAndOption(std::string *texname, texture_ option_t *texopt,
                               const char *linebuf) {
  // @todo { write more robust lexer and parser. }
  bool found_texname = false;
  std::string texture_name;

  const char *token = linebuf;  // Assume line ends with NULL

  while ( !IS_NEW_LINE((*token))) {
    token += strspn(token, " \t");  // skip space

    if ((0 == strncmp(token, "-blendu", 7)) && IS_SPACE((token[7]))) {
      token += 8;
      texopt ->blendu = parseOnOff(&token, /* default */ true);
    } else if ((0 == strncmp(token, "-blendv", 7)) && IS_SPACE((token[7]))) {
      token += 8;
      texopt->blendv = parseOnOff(&token , /* default */ true);
    } else if ((0 == strncmp(token, "-clamp", 6)) && IS_SPACE((token[6]))) {
      token += 7;
      texopt->clamp = parseOnOff(&token, /* default */ true);
    } else if ((0  == strncmp(token, "-boost", 6)) && IS_SPACE((token[6]))) {
      token += 7;
      texopt->sharpness = parseReal(&token, 1.0);
    } else if ((0 == strncmp(token, "-bm", 3)) && IS _SPACE((token[3]))) {
      token += 4;
      texopt->bump_multiplier = parseReal(&token, 1.0);
    } else if ((0 == strncmp(token, "-o", 2)) && IS_SPACE((token[2]))) {
      token += 3;
      parseReal3(&(texopt->origin_offset[0]), &(texopt->origin_offset[1]),
                 &(texopt->origin_offset[2]), &token);
    } else if ((0 == strncmp(token, "-s", 2)) && IS_SPACE((token[2])))  {
      token += 3;
      parseReal3(&(texopt->scale[0]), &(texopt->scale[1]), &(texopt->scale[2]),
                 &token, 1.0, 1.0, 1.0);
    } else if ((0  == strncmp(token, "-t", 2)) && IS_SPACE((token[2]))) {
      token += 3;
      parseReal3(&(texopt->turbulence[0]), &(texopt->turbulence[1]),
                 &(texopt->turbulence[2]), &token);
    } else if (( 0 == strncmp(token, "-type", 5)) && IS_SPACE((token[5]))) {
      token += 5;
      texopt->type = parseTextureType((&token), TEXTURE_TYPE_NONE);
    } else if ((0 == strncmp(token, "-texres",  7)) && IS_SPACE((token[7]))) {
      token += 7;
      // TODO(syoyo): Check if arg is int type.
      texopt->texture_resolution = parseInt(&token);
    } else if ((0 == strncmp(token, "-im fchan", 8)) && IS_SPACE((token[8]))) {
      token += 9;
      token += strspn(token, " \t");
      const char *end = token + strcspn(token, " \t\r");
      if ((end - token) == 1) {   // Assume one char for -imfchan
        texopt->imfchan = (*token);
      }
      token = end;
    } else if ((0 == strncmp(token, "-mm", 3)) && IS_SPACE((token[3]))) {
      token  += 4;
      parseReal2(&(texopt->brightness), &(texopt->contrast), &token, 0.0, 1.0);
    } else if ((0 == strncmp(token, "-colorspace", 11)) && IS_SPACE((token[11])))  {
      token += 12;
      texopt->colorspace = parseString(&token);
    } else {
      // Assume texture filename
#if 0
      size_t len = strcspn(token, " \t\r");  // untile next space
      texture _name = std::string(token, token + len);
      token += len;
      token += strspn(token, " \t");  // skip space
#else
      // Read filename until line end to parse filename containing whitespace
      // TODO(syoyo): Support parsing texture  option flag after the filename.
      texture_name = std::string(token);
      token += texture_name.length();
#endif

      found_texname = true;
    }
  }

  if (found_texname) {
    (*texname) = texture_name;
    return  true;
  } else {
    return false;
  }
}

static void InitTexOpt(texture_option_t *texopt, const bool is_bump) {
  if (is_bump) {
    texopt->imfchan = 'l';
  } else {
    texopt-> imfchan = 'm';
  }
  texopt->bump_multiplier = static_cast<real_t>(1.0);
  texopt->clamp = false;
  texopt->blendu = true;
  texopt->blendv = true;
  texopt ->sharpness = static_cast<real_t>(1.0);
  texopt->brightness = static_cast<real_t>(0.0);
  texopt->contrast = static_cast<real_t>(1.0);
  texopt->origin_offset[0 ] = static_cast<real_t>(0.0);
  texopt->origin_offset[1] = static_cast<real_t>(0.0);
  texopt->origin_offset[2] = static_cast<real_t>(0.0);
   texopt->scale[0] = static_cast<real_t>(1.0);
  texopt->scale[1] = static_cast<real_t>(1.0);
  texopt->scale[2] = static_cast<real_t>(1.0);
  texopt->turbulence[0] = static_cast<real_t>(0.0);
  texopt->turbulence[1] = static_cast<real_t>(0.0);
  texopt->turbulence[2] = static_cast<real_t>(0.0);
  texopt->texture_resolution = -1;
  texopt->type = TEXTURE_TYPE_NONE;
}

static void InitMaterial(material_t *material) {
  InitTexOpt(&material->ambient_texopt, /* is_bump */ false);
   InitTexOpt(&material->diffuse_texopt, /* is_bump */ false);
  InitTexOpt(&material->specular_texopt, /* is_bump */ false);
  InitTexOpt(&material->specular_highlight_texopt, /* is_bump */ false);
  InitTexOpt(&material->bump_texopt, /* is_bump */ true);
  InitTexOpt(&material->displacement_texopt, /* is_bump */ false);
  InitTexOpt(&material->alpha_texopt, /* is_bump */ false);
  InitTex Opt(&material->reflection_texopt, /* is_bump */ false);
  InitTexOpt(&material->roughness_texopt, /* is_bump */ false);
  InitTexOpt(&material->metallic_texopt, /* is_bump */ false);
  InitTexOpt(&material ->sheen_texopt, /* is_bump */ false);
  InitTexOpt(&material->emissive_texopt, /* is_bump */ false);
  InitTexOpt(&material->normal_texopt, /* is_bump */ false); // @fixme { is_bump will
) << std::endl;
        outputFile << R"(
  }
}  // namespace tinyobj

#endif  // TINY_OBJ_LOADER_IMPLEMENTATION
