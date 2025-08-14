/*****************************************************************************************
 *              MIT License                                                              *
 *                                                                                       *
 * Copyright (c) 2022 G. Cherchi, F. Pellacini, M. Attene and M. Livesu                  *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION     *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                *
 *                                                                                       *
 * Authors:                                                                              *
 *      Gianmarco Cherchi (g.cherchi@unica.it)                                           *
 *      https://www.gianmarcocherchi.com                                                 *
 *                                                                                       *
 *      Fabio Pellacini (fabio.pellacini@uniroma1.it)                                    *
 *      https://pellacini.di.uniroma1.it                                                 *
 *                                                                                       *
 *      Marco Attene (marco.attene@ge.imati.cnr.it)                                      *
 *      https://www.cnr.it/en/people/marco.attene/                                       *
 *                                                                                       *
 *      Marco Livesu (marco.livesu@ge.imati.cnr.it)                                      *
 *      http://pers.ge.imati.cnr.it/livesu/                                              *
 *                                                                                       *
 * ***************************************************************************************/

#ifdef _MSC_VER // Workaround for known bugs and issues on MSVC
#define _HAS_STD_BYTE 0  // https://developercommunity.visualstudio.com/t/error-c2872-byte-ambiguous-symbol/93889
#define NOMINMAX // https://stackoverflow.com/questions/1825904/error-c2589-on-stdnumeric-limitsdoublemin
#endif


#include "booleans.h"
#include <cinolib/color.h>
#include <array>

std::vector<std::string> files;

/*
 * USAGE AND LABEL ENCODING NOTES
 *
 * Each input mesh is assigned a unique label (0, 1, 2, ..., N-1 for N input meshes).
 * These labels are mapped to bits in a std::bitset<NBIT> for each output triangle.
 * For example, with 5 input meshes, the bits represent:
 *   bit 0: mesh 0  (value 1)
 *   bit 1: mesh 1  (value 2)
 *   bit 2: mesh 2  (value 4)
 *   bit 3: mesh 3  (value 8)
 *   bit 4: mesh 4  (value 16)
 * For each output triangle, the corresponding bitset in bool_labels encodes which
 * input mesh(es) the triangle is inside or derived from. For example:
 *   00001 (1)   => inside mesh 0 only
 *   00010 (2)   => inside mesh 1 only
 *   00100 (4)   => inside mesh 2 only
 *   00011 (3)   => inside mesh 0 and mesh 1 (overlap/intersection)
 *   11111 (31)  => inside all 5 meshes
 *
 * This allows you to interpret the provenance of each triangle in the boolean result.
 */

int main(int argc, char **argv)
{
    BoolOp op;
    std::string file_out;

    if(argc < 5)
    {
        std::cout << "syntax error!" << std::endl;
        std::cout << "./exact_boolean BOOL_OPERATION (intersection OR union OR subtraction OR xor OR nonreg) input1.obj input2.obj output.obj" << std::endl;
        return -1;
    }
    else
    {
        if (strcmp(argv[1], "intersection") == 0)       op = INTERSECTION;
        else if (strcmp(argv[1], "union") == 0)         op = UNION;
        else if (strcmp(argv[1], "subtraction") == 0)   op = SUBTRACTION;
        else if (strcmp(argv[1], "xor") == 0)           op = XOR;
        else if (strcmp(argv[1], "nonreg") == 0)        op = NONREG;
    }

    for(int i = 2; i < (argc -1); i++)
        files.emplace_back(argv[i]);

    file_out = argv[argc-1];

    std::vector<double> in_coords, bool_coords;
    std::vector<uint> in_tris, bool_tris;
    std::vector<uint> in_labels;
    std::vector<std::bitset<NBIT>> bool_labels;

    loadMultipleFiles(files, in_coords, in_tris, in_labels);

    booleanPipeline(in_coords, in_tris, in_labels, op, bool_coords, bool_tris, bool_labels);

    // Color lookup table for up to 10 input meshes (diverse, visually distinct)
    std::vector<cinolib::Color> face_colors;
    if (files.size() <= 10)
    {
        std::array<cinolib::Color, 10> color_lut = {
            cinolib::Color::RED(),
            cinolib::Color::GREEN(),
            cinolib::Color::BLUE(),
            cinolib::Color::YELLOW(),
            cinolib::Color::MAGENTA(),
            cinolib::Color::CYAN(),
            cinolib::Color::PASTEL_ORANGE(),
            cinolib::Color::PASTEL_VIOLET(),
            cinolib::Color::PASTEL_GREEN(),
            cinolib::Color::PASTEL_PINK()
        };

        face_colors.reserve(bool_labels.size());
        for(const auto& bits : bool_labels) {
            std::vector<cinolib::Color> active;
            for(size_t i=0; i<color_lut.size(); ++i) {
                if(i < bits.size() && bits[i]) active.push_back(color_lut[i]);
            }
            if(active.empty()) {
                face_colors.push_back(cinolib::Color::GRAY());
            } else if(active.size() == 1) {
                face_colors.push_back(active[0]);
            } else {
                float r=0, g=0, b=0;
                for(const auto& c : active) { r+=c.r; g+=c.g; b+=c.b; }
                float n = static_cast<float>(active.size());
                face_colors.push_back(cinolib::Color(r/n, g/n, b/n));
            }
        }
    }

    cinolib::write_OBJ(file_out.c_str(), bool_coords, bool_tris, std::vector<uint>{}, face_colors);

    return 0;
}