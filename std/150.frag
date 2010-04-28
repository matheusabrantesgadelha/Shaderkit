/**
 * Copyright 2010 Riku Palomäki.
 * This file is part of GLSL Lab.
 *
 * GLSL Lab is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GLSL Lab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLSL Lab.  If not, see <http://www.gnu.org/licenses/>.
 */

in vec4 gl_FragCoord;
in bool gl_FrontFacing;
in float gl_ClipDistance[];
out vec4 gl_FragColor; // deprecated
out vec4 gl_FragData[gl_MaxDrawBuffers]; // deprecated
out float gl_FragDepth;
in vec2 gl_PointCoord;
in int gl_PrimitiveID;

// Returns the derivative in x using local differencing for the input argument p.
genType dFdx(genType p);
// Returns the derivative in y using local differencing for the input argument p.
genType dFdy(genType p);
// Returns the sum of the absolute derivative in x and y using local differencing for the input argument p.
genType fwidth(genType p);
