
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace nyanchu {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && texcoord == other.texcoord;
    }
};
} // namespace nyanchu

namespace std {
    template<> struct hash<nyanchu::Vertex> {
        size_t operator()(nyanchu::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                   (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texcoord) << 1);
        }
    };
}

namespace nyanchu {

class Mesh {
public:
    Mesh(const std::string& filepath);

    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }

private:
    void loadFromFile(const std::string& filepath);

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

} // namespace nyanchu
