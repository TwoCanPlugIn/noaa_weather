// Copyright(C) 2018-2020 by Steven Adler
//
// This file is part of NOAA Weather plugin for OpenCPN.
//
// NOAA Weather plugin for OpenCPN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NOAA Weather plugin for OpenCPN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the NOAA Weather plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//

#ifndef NOAA_GRAPHICS_H
#define NOAA_GRAPHICS_H

// pIDC implements a layer on top of OpenGL
#include "pidc.h"

class NOAA_Graphics : public piDC {
private:
    double scaleFactor = 1.0f;
    bool isOpenGL;

public:
    // OpenGL
    NOAA_Graphics(wxGLContext* context) : piDC(context) {
        glcontext = context;
        isOpenGL = true;
    };
   
    // non OpenGL
    NOAA_Graphics(wxDC& pdc): piDC(pdc) {
        dc = &pdc;
        scaleFactor = pdc.GetContentScaleFactor();
        isOpenGL = false;
    };

  
    NOAA_Graphics() : piDC() {
        isOpenGL = true;
    };

    bool IsGL() {
        return isOpenGL;
    };

    double GetContentScaleFactor() const {
        return scaleFactor;
    };

    void SetContentScaleFactor(double factor) { 
        scaleFactor = factor; 
    };

    bool CheckContext(wxGLContext* context) const {
        return glcontext == context;
    }
};

#endif