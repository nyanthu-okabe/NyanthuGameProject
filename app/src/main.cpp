#include "application.h"

/*
 * Nyanthu Okabe 2025-12-25
 *
 * Copyright (c) 2025 nyanthu.com
 * All rights reserved.
 *
 * Do not modify or copy without permission.
 */


int main()
{
    Application app;
    if (!app.initialize())
    {
        return -1;
    }

    app.run();

    return 0;
}
