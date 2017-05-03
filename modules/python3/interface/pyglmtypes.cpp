/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/


#include <modules/python3/interface/pyglmtypes.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <inviwo/core/util/stringconversion.h>

#include <map>
#include <string>
#include <inviwo/core/util/logcentral.h>

#include <pybind11/operators.h>

#include <algorithm>


namespace py = pybind11;

template <typename T, size_t N>
using ith_T = T;

template <typename T, typename V, unsigned C, typename Indices = std::make_index_sequence<C>>
void addInit(py::class_<V> &pyv) {
    addInitImpl<V, T>(pyv, Indices{});
}

template <typename V, typename T, std::size_t... I>
void addInitImpl(py::class_<V> &pyv, std::index_sequence<I...>) {
    pyv.def(py::init<ith_T<T, I>...>());
}

namespace inviwo {

    template <typename T,typename GLM> void common(py::module &m,py::class_<GLM> &pyc , std::string name){
        pyc
            .def(py::init<T>())
            .def(py::init<>())
            .def("__repr__", [](GLM &v) { return glm::to_string(v); })

            .def(py::self + py::self)
            .def(py::self - py::self)
            .def(py::self += py::self)
            .def(py::self -= py::self)
            .def(py::self == py::self)
            .def(py::self != py::self)

            .def(py::self + T())
            .def(py::self - T())
            .def(py::self * T())
            .def(py::self / T())
            .def(py::self += T())
            .def(py::self -= T())
            .def(py::self *= T())
            .def(py::self /= T())

            .def("__getitem__", [](GLM &v, int idx) { return &v[idx]; } , py::return_value_policy::reference_internal)
            ;

        py::bind_vector<std::vector<GLM>>(m, name + "Vector");
    }

    template<typename T, typename V>
    void floatOnlyVecs(py::module &m,  std::false_type) {}

    template<typename T, typename V>
    void floatOnlyVecs(py::module &m, std::true_type) {
        //        p.def("cross", [](V &v, V &v2) { return glm::cross(v, v2); });
        m.def("dot", [](V &v, V &v2) { return glm::dot(v, v2); });
        m.def("distance", [](V &v, V &v2) { return glm::distance(v, v2); });
        m.def("distance2", [](V &v, V &v2) { return glm::distance2(v, v2); });
        m.def("length", [](V &v) { return glm::length(v); });
        m.def("length2", [](V &v) { return glm::length2(v); });
        m.def("normalize", [](V &v) { return glm::normalize(v); });
    }

    template<typename T, typename V>
    void floatOnlyVecs(py::module &m ) {
        floatOnlyVecs<T,V>(m,std::is_floating_point<T>());
    }


    template <typename T,unsigned A>
    void vecx(py::module &m, std::string prefix, std::string name = "vec", std::string postfix = "") {
        using V = Vector<A, T>;
        std::stringstream classname;
        classname << prefix << name << A << postfix;
        py::class_<V> pyv(m, classname.str().c_str());
        common<T>(m,pyv,classname.str());
        addInit<T, V, A>(pyv);
        pyv 
            .def(py::self * py::self)
            .def(py::self / py::self)
            .def(py::self *= py::self)
            .def(py::self /= py::self)

            .def("__str__",
                [](V &v) {
                    std::ostringstream oss;
                    oss << "[";
                    for (int i = 0; i < A; i++) {
                        if (i != 0) {
                            oss << " ";
                        }
                        oss << v[i];
                    }
                    oss << "]";
                    return oss.str();
        })

            .def("__setitem__", [](V &v, int idx, T &t) { return v[idx] = t; });
        floatOnlyVecs<T,V>(m);
    }

    

    template <typename T>
    void vec(py::module &m, std::string prefix , std::string name = "vec" , std::string postfix = "") {
        vecx<T, 2>(m, prefix, name, postfix);
        vecx<T, 3>(m, prefix, name, postfix);
        vecx<T, 4>(m, prefix, name, postfix);
    }


    template <typename T, unsigned COLS, unsigned ROWS>
    void matxx(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
        using M = typename util::glmtype<T, COLS, ROWS>::type;
        using M2 = typename util::glmtype<T, 2, ROWS>::type;
        using M3 = typename util::glmtype<T, 3, ROWS>::type;
        using M4 = typename util::glmtype<T, 4, ROWS>::type;
        using Ma2 = typename util::glmtype<T,  ROWS,2>::type;
        using Ma3 = typename util::glmtype<T,  ROWS,3>::type;
        using Ma4 = typename util::glmtype<T,  ROWS,4>::type;
        using Va = Vector<COLS, T>;
        using Vb = Vector<ROWS, T>;

        std::stringstream classname;
        classname << prefix << name << COLS;
        if (COLS != ROWS) {
            classname << "x" << ROWS;
        }

        py::class_<M> pym(m, classname.str().c_str());
        common<T>(m,pym,classname.str());
        addInit<T, M, COLS*ROWS>(pym);
        addInit<typename M::col_type, M, ROWS>(pym);
        pym.def(py::self * Vb())
            .def(py::self * Ma2())
            .def(py::self * Ma3())
            .def(py::self * Ma4())

            .def("__getitem__", [](M &m, int idx, int idy) { return m[idx][idy]; })
            .def("__setitem__", [](M &m, int idx, Va &t) { return m[idx] = t; })
            .def("__setitem__", [](M &m, int idx, int idy, T &t) { return m[idx][idy] = t; })

            .def("__str__",
                 [](M &m) {
                     std::ostringstream oss;
                     oss << "[";
                     for (int col = 0; col < COLS; col++) {
                         if (col != 0) {
                             oss << ",";
                         }
                         oss << "[";
                         for (int row = 0; row < ROWS; row++) {
                             if (row != 0) {
                                 oss << " ";
                             }
                             oss << m[col][row];
                         }
                         oss << "]";
                     }
                     oss << "]";
                     return oss.str();
                 })

            //            .def("transpose", [](M &m) {return glm::transpose(m); })
            //            .def("inverse", [](M &m) {return glm::inverse(m); })
            //            .def("determinant", [](M &m) {return glm::determinant(m); })
            ;
    }

    template <typename T, unsigned COLS>
    void matx(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
        matxx<T, COLS, 2>(m, prefix, name, postfix);
        matxx<T, COLS, 3>(m, prefix, name, postfix);
        matxx<T, COLS, 4>(m, prefix, name, postfix);
    }

    template <typename T>
    void mat(py::module &m, std::string prefix, std::string name = "mat", std::string postfix = "") {
        matx<T, 2>(m, prefix, name, postfix);
        matx<T, 3>(m, prefix, name, postfix);
        matx<T, 4>(m, prefix, name, postfix);
    }


    template <typename T>
    void glmtypes(py::module &m, std::string prefix, std::string postfix = "" ) {
        vec<T>(m, prefix, "vec", postfix);
        mat<T>(m, prefix, "mat", postfix);
    }

    template <typename B>
    struct A : public B{};

    void exposeGLMTypes(py::module &m) {
        auto glmModule = m.def_submodule("glm", "Exposing glm vec and mat types");

        glmtypes<float>(glmModule, ""); 
        glmtypes<double>(glmModule, "d");
        glmtypes<int>(glmModule, "i");
        glmtypes<unsigned int>(glmModule, "u");
        vec<size_t>(glmModule, "","size", "_t");
    }


}