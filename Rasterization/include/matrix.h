#pragma once
#include <iostream>
#include "vector.h"

namespace lxrr {

enum class MatrixScale {
    m_2x2 = 2,
    m_3x3 = 3,
    m_4x4 = 4
};

template<MatrixScale m_size>
class Matrix {
public:
    Matrix();

    template<MatrixScale o_m_size>
    Matrix(const Matrix<o_m_size>& other);
    Matrix<m_size>& operator=(const Matrix<MatrixScale::m_3x3>& other);

    ~Matrix() = default;

    float operator()(int i, int j) const { return m[i][j]; }
    float& operator()(int i, int j) { return m[i][j]; }
    Matrix<MatrixScale::m_3x3> get_3x3() const;
    void set_3x3(const Matrix<MatrixScale::m_3x3>& other);

    Matrix<m_size> operator+(const Matrix<m_size>& right) const;
    Matrix<m_size> operator-(const Matrix<m_size>& right) const;
    Matrix<m_size> operator*(const Matrix<m_size>& right) const;
    Matrix<m_size> operator*(float t) const;
    template<MatrixScale t_m_size> friend Matrix<t_m_size> operator*(float t, const Matrix<t_m_size>& right);
    template<class T = vec3>
    T operator*(const T& v) const;

    void identity();

    Matrix<m_size> transpose() const;

    template<MatrixScale t_m_size> friend std::ostream& operator<<(std::ostream& out, const Matrix<t_m_size>& m);
private:
    static constexpr int get_size() { return static_cast<int>(m_size); }
    float m[static_cast<int>(m_size)][static_cast<int>(m_size)];
};

using mat2x2 = Matrix<MatrixScale::m_2x2>;
using mat3x3 = Matrix<MatrixScale::m_3x3>;
using mat4x4 = Matrix<MatrixScale::m_4x4>;




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Implement
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
template<MatrixScale m_size>
Matrix<m_size>::Matrix() {
    identity();
}

template<MatrixScale m_size>
template<MatrixScale o_m_size>
Matrix<m_size>::Matrix(const Matrix<o_m_size>& other) {
    if (other.get_size() < get_size()) {
        throw std::runtime_error("Matrix size is too large!");
    }

    for (int i = 0; i < get_size(); i++) {
        for (int j = 0; j < get_size(); j++) {
            m[i][j] = other(i,j);
        }
    }
}

template<MatrixScale m_size>
Matrix<m_size>& Matrix<m_size>::operator=(const Matrix<MatrixScale::m_3x3>& other) {
    if (m_size != MatrixScale::m_4x4 || m_size != MatrixScale::m_3x3) {
        throw std::runtime_error("Need 4x4 or 3x3 matrix!");
    }
    identity();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m[i][j] = other.m[i][j];
        }
    }
}


template<MatrixScale m_size>
Matrix<MatrixScale::m_3x3> Matrix<m_size>::get_3x3() const {
    if (m_size != MatrixScale::m_4x4) {
        throw std::runtime_error("Calling get_3x3() need 4x4 matrix!");
    }
    Matrix<MatrixScale::m_3x3> result;
    for (int i = 0; i < 3; i ++) {
        for (int j = 0; j < 3; j ++) {
            result(i,j) = m[i][j];
        }
    }
    return result;
}

template<MatrixScale m_size>
void Matrix<m_size>::set_3x3(const Matrix<MatrixScale::m_3x3>& other) {
    if (m_size != MatrixScale::m_4x4) {
        throw std::runtime_error("Calling get_3x3() need 4x4 matrix!");
    }
    for (int i = 0; i < 3; i ++) {
        for (int j = 0; j < 3; j ++) {
            m[i][j] = other(i,j);
        }
    }
}

template<MatrixScale m_size>
Matrix<m_size> Matrix<m_size>::operator+(const Matrix<m_size>& right) const {
    Matrix<m_size> result;
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            result(i,j) = m[i][j] + right(i,j);
        }
    }
    return result;
}

template<MatrixScale m_size>
Matrix<m_size> Matrix<m_size>::operator-(const Matrix<m_size>& right) const {
    Matrix<m_size> result;
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            result(i,j) = m[i][j] - right(i,j);
        }
    }
    return result;
}

template<MatrixScale m_size>
Matrix<m_size> Matrix<m_size>::operator*(const Matrix<m_size>& right) const {
    Matrix<m_size> result;
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            result(i,j) = 0;
            for (int k = 0; k < get_size(); k ++) {
                result(i,j) += m[i][k] * right(k,j);
            }
        }
    }
    return result;
}

template<MatrixScale m_size>
Matrix<m_size> Matrix<m_size>::operator*(float t) const {
    Matrix<m_size> result;
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            result(i,j) = m[i][j] * t;
        }
    }
    return result;
}

template<MatrixScale m_size>
Matrix<m_size> operator*(float t, const Matrix<m_size>& right) {
    Matrix<m_size> result;
    result = right * t;
    return result;
}

template<MatrixScale m_size>
template<class T>
T Matrix<m_size>::operator*(const T& v) const {
    T result;
    for (int i = 0; i < get_size(); i ++) {
        result[i] = 0;
        for (int j = 0; j < get_size(); j ++) {
            result[i] += m[i][j] * v[j];
        }
    }
    return result;
}

template<MatrixScale m_size>
void Matrix<m_size>::identity() {
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            m[i][j] = i == j ? 1 : 0;
        }
    }
}

template<MatrixScale m_size>
Matrix<m_size> Matrix<m_size>::transpose() const {
    Matrix<m_size> result;
    for (int i = 0; i < get_size(); i ++) {
        for (int j = 0; j < get_size(); j ++) {
            result(i,j) = m[j][i];
        }
    }
    return result;
}

template<MatrixScale m_size>
std::ostream& operator<<(std::ostream& out, const Matrix<m_size>& m) {
    out << "[";
    for (int i = 0; i < Matrix<m_size>::get_size(); i ++) {
        out << "[";
        for (int j = 0; j < Matrix<m_size>::get_size(); j ++) {
            out << m(i,j);
            if (j != Matrix<m_size>::get_size() - 1) out << ",";
        }
        out << (i != Matrix<m_size>::get_size() - 1 ? "]\n " : "]");
    }
    out << "]";
    return out;
}

} // namespace lxrr