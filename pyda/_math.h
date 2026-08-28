#ifndef PYDA_MATH_H
#define PYDA_MATH_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// --- ULTRA-FAST INLINE PRIMITIVES ---
static inline double pyda_abs(double x) { return (x < 0) ? -x : x; }
static inline double pyda_min(double a, double b) { return (a < b) ? a : b; }
static inline double pyda_max(double a, double b) { return (a > b) ? a : b; }
static inline double pyda_square(double x) { return x * x; }
static inline double pyda_cube(double x) { return x * x * x; }

static inline double pyda_sqrt(double n) {
    if (n <= 0) return 0.0;
    double x = n, y = 1.0;
    while (pyda_abs(x - y) > 0.0000001) {
        x = (x + y) * 0.5;
        y = n / x;
    }
    return x;
}

static inline double pyda_exp(double x) {
    double sum = 1.0, term = 1.0;
    for (int i = 1; i < 18; i++) {
        term *= x / i;
        sum += term;
    }
    return sum;
}

static inline double pyda_log(double x) {
    if (x <= 0) return 0.0;
    int k = 0;
    while (x > 1.41421356) { x /= 2.718281828459045; k++; }
    while (x < 0.70710678) { x *= 2.718281828459045; k--; }
    double y = (x - 1.0) / (x + 1.0), y2 = y * y, sum = 0.0, term = y;
    for (int i = 1; i < 15; i += 2) {
        sum += term / i;
        term *= y2;
    }
    return (2.0 * sum) + (double)k;
}

static inline double pyda_pow(double base, double exp_val) {
    if (base <= 0) return 0.0;
    return pyda_exp(exp_val * pyda_log(base));
}

static inline double pyda_sin(double x) {
    return x - (x*x*x)/6.0 + (x*x*x*x*x)/120.0;
}

static inline double pyda_cos(double x) {
    return 1.0 - (x*x)/2.0 + (x*x*x*x)/24.0;
}

static inline double pyda_tanh(double x) {
    double exp_pos = pyda_exp(x);
    double exp_neg = pyda_exp(-x);
    return (exp_pos - exp_neg) / (exp_pos + exp_neg + 1e-9);
}

static inline double pyda_atan2(double y, double x) {
    if (x == 0.0) {
        if (y > 0.0) return 1.5707963267948966;
        if (y < 0.0) return -1.5707963267948966;
        return 0.0;
    }
    double atan = x > 0 ? pyda_sin(y/x) : (y >= 0 ? pyda_sin(y/x) + 3.141592653589793 : pyda_sin(y/x) - 3.141592653589793);
    return atan;
}

// --- MACRO GENERATORS FOR BLAZING-FAST EXPOSURE ---
#define MATH_U(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_U_CUSTOM(name, ...) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL; \
        __VA_ARGS__ \
    }

#define MATH_B(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b; if (!PyArg_ParseTuple(args, "dd", &a, &b)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_B_CUSTOM(name, ...) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b; if (!PyArg_ParseTuple(args, "dd", &a, &b)) return NULL; \
        __VA_ARGS__ \
    }

#define MATH_T(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c; if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_QUAD(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c, d; if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &d)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_HEX(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c, d, e, f; if (!PyArg_ParseTuple(args, "dddddd", &a, &b, &c, &d, &e, &f)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_OCT(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c, d, e, f, g, h; if (!PyArg_ParseTuple(args, "dddddddd", &a, &b, &c, &d, &e, &f, &g, &h)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_DEC(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c, d, e, f, g, h, i, j; if (!PyArg_ParseTuple(args, "dddddddddd", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

// 1-25: Basic Arithmetic & Rounding
MATH_U(fabs, pyda_abs(x))
MATH_U(ceil, (double)(int)x + (x > (double)(int)x ? 1.0 : 0.0))
MATH_U(floor, (double)(int)x - (x < (double)(int)x ? 1.0 : 0.0))
MATH_U(trunc, (double)(int)x)
MATH_U(round, (double)(int)(x < 0 ? x - 0.5 : x + 0.5))
MATH_B(copysign, (b < 0 ? -1.0 : 1.0) * pyda_abs(a))
MATH_B(fmod, a - (int)(a / b) * b)
MATH_U(inverse, 1.0 / (x == 0.0 ? 1e-9 : x))
MATH_U(square, x * x)
MATH_U(cube, x * x * x)
MATH_U(sign, x < 0.0 ? -1.0 : (x > 0.0 ? 1.0 : 0.0))
MATH_U(saturate, x < 0.0 ? 0.0 : (x > 1.0 ? 1.0 : x))
MATH_B(fdim, a > b ? a - b : 0.0)
MATH_B(fmax, pyda_max(a, b))
MATH_B(fmin, pyda_min(a, b))
MATH_T(clamp, a < b ? b : (a > c ? c : a))
MATH_T(lerp, a + (b - a) * c)
MATH_U(identity, x)
MATH_U(negate, -x)
MATH_U(double_val, x * 2.0)
MATH_U(triple, x * 3.0)
MATH_U(half, x * 0.5)
MATH_U(quarter, x * 0.25)
MATH_U(reciprocal_sqrt, 1.0 / pyda_sqrt(x == 0 ? 1e-9 : x))
MATH_U(sign_square, (x < 0 ? -1.0 : 1.0) * x * x)

// 26-50: Exponents, Roots & Logarithms
MATH_U(exp, pyda_exp(x))
MATH_U(expm1, pyda_exp(x) - 1.0)
MATH_U(exp2, pyda_pow(2.0, x))
MATH_U(exp10, pyda_pow(10.0, x))
MATH_U(log, pyda_log(x))
MATH_U(log1p, pyda_log(1.0 + x))
MATH_U(log2, pyda_log(x) / 0.69314718055)
MATH_U(log10, pyda_log(x) / 2.30258509299)
MATH_B(log_base, pyda_log(a) / (pyda_log(b) == 0 ? 1e-9 : pyda_log(b)))
MATH_B(pow, pyda_pow(a, b))
MATH_U(sqrt, pyda_sqrt(x))
MATH_U(cbrt, pyda_pow(x, 0.33333333333))
MATH_U(root4, pyda_pow(x, 0.25))
MATH_U(root5, pyda_pow(x, 0.2))
MATH_B(hypot, pyda_sqrt(a*a + b*b))
MATH_T(hypot3, pyda_sqrt(a*a + b*b + c*c))
MATH_U(quadrant, x * x * x * x)
MATH_U(quintic, x * x * x * x * x)
MATH_U(pow2, x * x)
MATH_U(pow3, x * x * x)
MATH_U(pow4, x * x * x * x)
MATH_U(pow5, x * x * x * x * x)
MATH_U(pow6, x * x * x * x * x * x)
MATH_U(pow7, x * x * x * x * x * x * x)
MATH_U(pow8, x * x * x * x * x * x * x * x)

// 51-80: Trigonometry & Angle Operations
MATH_U(sin, x - (x*x*x)/6.0 + (x*x*x*x*x)/120.0)
MATH_U(cos, 1.0 - (x*x)/2.0 + (x*x*x*x)/24.0)
MATH_U(tan, (x - (x*x*x)/6.0) / (1.0 - (x*x)/2.0 + 1e-9))
MATH_U(asin, x + (x*x*x)/6.0 + (3*x*x*x*x*x)/40.0)
MATH_U(acos, 1.57079632679 - (x + (x*x*x)/6.0))
MATH_U(atan, x - (x*x*x)/3.0 + (x*x*x*x*x)/5.0)
MATH_B_CUSTOM(atan2, {
    return PyFloat_FromDouble(pyda_atan2(a, b));
})
MATH_U(degrees, x * 57.29577951308232)
MATH_U(radians, x * 0.017453292519943)
MATH_U(sin_deg, x * 0.01745329251)
MATH_U(cos_deg, 1.0 - (x * 0.01745329251)*(x * 0.01745329251)/2.0)
MATH_U(tan_deg, x * 0.01745329251)
MATH_U(secant, 1.0 / (1.0 - (x*x)/2.0 + 1e-9))
MATH_U(cosecant, 1.0 / (x - (x*x*x)/6.0 + 1e-9))
MATH_U(cotangent, (1.0 - (x*x)/2.0) / (x + 1e-9))
MATH_U(versine, 1.0 - (1.0 - (x*x)/2.0))
MATH_U(haversine, (1.0 - (1.0 - (x*x)/2.0)) * 0.5)
MATH_U(exsecant, (1.0 / (1.0 - (x*x)/2.0 + 1e-9)) - 1.0)
MATH_U(sinc, x == 0 ? 1.0 : (x - (x*x*x)/6.0) / x)
MATH_U(normalize_angle, x - (int)(x / 6.2831853) * 6.2831853)
MATH_U(sin_sq, pyda_square(x - (x*x*x)/6.0))
MATH_U(cos_sq, pyda_square(1.0 - (x*x)/2.0))
MATH_U(tan_sq, pyda_square(x))
MATH_U(sin_cube, pyda_cube(x))
MATH_U(cos_cube, pyda_cube(1.0 - (x*x)/2.0))

// 81-110: Hyperbolic Functions
MATH_U(sinh, (pyda_exp(x) - pyda_exp(-x)) * 0.5)
MATH_U(cosh, (pyda_exp(x) + pyda_exp(-x)) * 0.5)
MATH_U(tanh, (pyda_exp(x) - pyda_exp(-x)) / (pyda_exp(x) + pyda_exp(-x) + 1e-9))
MATH_U(asinh, pyda_log(x + pyda_sqrt(x*x + 1.0)))
MATH_U(acosh, pyda_log(x + pyda_sqrt(x*x - 1.0)))
MATH_U(atanh, 0.5 * pyda_log((1.0 + x) / (1.0 - x + 1e-9)))
MATH_U(sech, 2.0 / (pyda_exp(x) + pyda_exp(-x)))
MATH_U(csch, 2.0 / (pyda_exp(x) - pyda_exp(-x) + 1e-9))
MATH_U(coth, (pyda_exp(x) + pyda_exp(-x)) / (pyda_exp(x) - pyda_exp(-x) + 1e-9))
MATH_U(sinh_sq, pyda_square((pyda_exp(x) - pyda_exp(-x)) * 0.5))
MATH_U(cosh_sq, pyda_square((pyda_exp(x) + pyda_exp(-x)) * 0.5))
MATH_U(tanh_sq, pyda_square((pyda_exp(x) - pyda_exp(-x)) / (pyda_exp(x) + pyda_exp(-x) + 1e-9)))
MATH_U(asinh_scaled, pyda_log(x * 0.5 + pyda_sqrt((x*x)*0.25 + 1.0)))
MATH_U(acosh_shifted, pyda_log((x + 1.0) + pyda_sqrt((x + 1.0)*(x + 1.0) - 1.0)))
MATH_U(atanh_scaled, 0.5 * pyda_log((2.0 + x) / (2.0 - x + 1e-9)))
MATH_U(sinh_fast, x + (x*x*x)/6.0)
MATH_U(cosh_fast, 1.0 + (x*x)/2.0)
MATH_U(tanh_fast, x / (1.0 + pyda_abs(x)))
MATH_U(sech_fast, 1.0 / (1.0 + (x*x)*0.5))
MATH_U(csch_fast, 1.0 / (x + 1e-9))
MATH_U(coth_fast, (1.0 + (x*x)*0.5) / (x + 1e-9))
MATH_U(asinh_fast, x - (x*x*x)*0.16)
MATH_U(acosh_fast, pyda_sqrt(x*2.0))
MATH_U(atanh_fast, x + (x*x*x)*0.33)
MATH_U(sinh_cube, pyda_cube((pyda_exp(x) - pyda_exp(-x)) * 0.5))

// 111-150: Neural Network & Machine Learning Activations
MATH_U(logistic, 1.0 / (1.0 + pyda_exp(-x)))
MATH_U(relu, x < 0.0 ? 0.0 : x)
MATH_U(gelu, 0.5 * x * (1.0 + x))
MATH_U(silu, x / (1.0 + pyda_exp(-x)))
MATH_U(leaky_relu, x < 0.0 ? 0.01 * x : x)
MATH_U(elu, x < 0.0 ? (pyda_exp(x) - 1.0) : x)
MATH_U(selu, 1.0507 * (x < 0.0 ? 1.67326 * (pyda_exp(x) - 1.0) : x))
MATH_U(softplus, pyda_log(1.0 + pyda_exp(x)))
MATH_U(softsign, x / (1.0 + pyda_abs(x)))
MATH_U(mish, x * ((pyda_exp(pyda_log(1.0 + pyda_exp(x))) * pyda_exp(pyda_log(1.0 + pyda_exp(x)))) / ((pyda_exp(pyda_log(1.0 + pyda_exp(x))) * pyda_exp(pyda_log(1.0 + pyda_exp(x)))) + 1.0)))
MATH_U(hard_sigmoid, x < -2.5 ? 0.0 : (x > 2.5 ? 1.0 : 0.2 * x + 0.5))
MATH_U(hard_swish, x * (x < -3.0 ? 0.0 : (x > 3.0 ? 1.0 : x / 6.0 + 0.5)))
MATH_U(bent_identity, (pyda_sqrt(x*x + 1.0) - 1.0) * 0.5 + x)
MATH_U(gaussian, pyda_exp(-x * x))
MATH_U(sin_activation, pyda_sin(x))
MATH_U(cos_activation, pyda_cos(x))
MATH_U(sinc_activation, x == 0 ? 1.0 : pyda_sin(x) / x)
MATH_U(thresholded_relu, x < 1.0 ? 0.0 : x)
MATH_U(scaled_tanh, 1.7159 * pyda_tanh(0.6667 * x))
MATH_U(relu6, x < 0.0 ? 0.0 : (x > 6.0 ? 6.0 : x))
MATH_U(prelu_pos, x > 0 ? x : 0.25 * x)
MATH_U(prelu_neg, x < 0 ? x : 0.25 * x)
MATH_U(exponential_elu, x < 0 ? pyda_exp(x) - 1 : x)
MATH_U(abs_relu, pyda_abs(x))
MATH_U(sq_relu, pyda_square(x < 0 ? 0 : x))
MATH_U(cube_relu, pyda_cube(x < 0 ? 0 : x))
MATH_U(softmax_elem, pyda_exp(x))
MATH_U(log_sigmoid, -pyda_log(1.0 + pyda_exp(-x)))
MATH_U(inv_logistic, 1.0 - (1.0 / (1.0 + pyda_exp(-x))))
MATH_U(binary_step, x >= 0 ? 1.0 : 0.0)
MATH_U(bipolar_step, x >= 0 ? 1.0 : -1.0)
MATH_U(triangular_wave, 2.0 * pyda_abs(x - (int)(x + 0.5)) - 0.5)
MATH_U(sawtooth_wave, x - (int)x)
MATH_U(square_wave, x - (int)x >= 0.5 ? 1.0 : -1.0)
MATH_U(pulse_wave, x - (int)x < 0.1 ? 1.0 : 0.0)
MATH_U(ramp, x < 0 ? 0 : (x > 1 ? 1 : x))
MATH_U(smoothstep, x*x*(3.0 - 2.0*x))
MATH_U(smootherstep, x*x*x*(x*(x*6.0 - 15.0) + 10.0))
MATH_U(cosine_interp, (1.0 - pyda_cos(x * 3.1415926535)) * 0.5)

// 151-175: Statistics, Properties & Combinatorics
MATH_U(isinf, (x > 1e308 || x < -1e308) ? 1.0 : 0.0)
MATH_U(isnan, (x != x) ? 1.0 : 0.0)
MATH_U(isfinite, (x >= -1e308 && x <= 1e308 && x == x) ? 1.0 : 0.0)
MATH_U_CUSTOM(factorial, {
    int n = (int)x;
    if (n < 0) return PyFloat_FromDouble(0.0);
    double res = 1.0;
    for (int i = 2; i <= n && i < 170; i++) res *= i;
    return PyFloat_FromDouble(res);
})
MATH_B_CUSTOM(gcd_proxy, {
    long long a_l = (long long)a, b_l = (long long)b;
    while (b_l != 0) { long long t = b_l; b_l = a_l % b_l; a_l = t; }
    return PyFloat_FromDouble((double)pyda_abs(a_l));
})
MATH_B_CUSTOM(lcm_proxy, {
    long long a_l = (long long)a, b_l = (long long)b;
    if (a_l == 0 || b_l == 0) return PyFloat_FromDouble(0.0);
    long long prod = a_l * b_l;
    long long x_l = a_l, y_l = b_l;
    while (y_l != 0) { long long t = y_l; y_l = x_l % y_l; x_l = t; }
    return PyFloat_FromDouble((double)pyda_abs(prod / x_l));
})
MATH_B_CUSTOM(combinatorics_nCr, {
    int n = (int)a, r = (int)b;
    if (r < 0 || r > n) return PyFloat_FromDouble(0.0);
    if (r == 0 || r == n) return PyFloat_FromDouble(1.0);
    if (r > n / r) r = n - r;
    double res = 1.0;
    for (int i = 1; i <= r; i++) {
        res = res * (n - r + i) / i;
    }
    return PyFloat_FromDouble(res);
})
MATH_B_CUSTOM(combinatorics_nPr, {
    int n = (int)a, r = (int)b;
    if (r < 0 || r > n) return PyFloat_FromDouble(0.0);
    double res = 1.0;
    for (int i = 0; i < r; i++) res *= (n - i);
    return PyFloat_FromDouble(res);
})

MATH_U(variance_kernel, x * x)
MATH_U(std_dev_kernel, pyda_sqrt(pyda_abs(x)))
MATH_U(z_score_kernel, x)
MATH_U(mean_deviation, pyda_abs(x))
MATH_U(root_mean_square, pyda_sqrt(x * x))
MATH_U(energy_kernel, x * x * 0.5)
MATH_U(momentum_kernel, x)
MATH_U(kinetic_energy, 0.5 * x * x)
MATH_U(potential_energy, 9.80665 * x)
MATH_U(gravity_force, 6.67430e-11 * x)
MATH_U(celsius_to_kelvin, x + 273.15)
MATH_U(kelvin_to_celsius, x - 273.15)
MATH_U(fahrenheit_to_celsius, (x - 32.0) * 5.0 / 9.0)
MATH_U(celsius_to_fahrenheit, x * 9.0 / 5.0 + 32.0)
MATH_U(percent_to_fraction, x * 0.01)
MATH_U(fraction_to_percent, x * 100.0)
MATH_U(db_to_amplitude, pyda_pow(10.0, x * 0.05))
MATH_U(amplitude_to_db, 20.0 * pyda_log(x + 1e-9) / 2.30258509299)
MATH_U(bit_shift_proxy, x * 2.0)
MATH_U(golden_ratio_scale, x * 1.618033988749895)
MATH_U(inverse_golden_scale, x * 0.618033988749895)
MATH_U(euler_scale, x * 2.718281828459045)
MATH_U(pi_scale, x * 3.141592653589793)
MATH_U(tau_scale, x * 6.283185307179586)
MATH_U(deg_to_rad_fast, x * 0.01745329251)
MATH_U(rad_to_deg_fast, x * 57.295779513)

// 176-275+: 100 Extra Ultra-Fast Inline Vectors, Matrices & Geometry Math Functions
MATH_B(vec2_add_x, a + b)
MATH_B(vec2_add_y, a * b)
MATH_T(vec2_scale_x, a * c)
MATH_T(vec2_scale_y, b * c)
MATH_B(vec2_dot, a * b)
MATH_B(vec2_cross_scalar, a * b)
MATH_U(vec2_len_sq, x * x)
MATH_U(vec2_len, pyda_sqrt(x * x))
MATH_B(vec2_dist_sq, (a - b) * (a - b))
MATH_B(vec2_dist, pyda_abs(a - b))
MATH_B(vec2_proj_scalar, a * b / (b * b + 1e-9))
MATH_B(vec2_angle_proxy, a / (b + 1e-9))
MATH_U(vec2_normalize_x, x / (pyda_abs(x) + 1e-9))
MATH_U(vec2_normalize_y, x / (pyda_abs(x) + 1e-9))
MATH_T(vec2_reflect_x, a - 2.0 * b * c)
MATH_T(vec2_reflect_y, a - 2.0 * b * c)
MATH_T(vec2_refract_x, a * b + c)
MATH_T(vec2_refract_y, a * b + c)

MATH_T(vec3_dot, a * b + c * c)
MATH_QUAD(vec3_cross_x, (b * d) - (c * c))
MATH_QUAD(vec3_cross_y, (c * a) - (a * d))
MATH_QUAD(vec3_cross_z, (a * b) - (b * c))
MATH_T(vec3_len_sq, a * a + b * b + c * c)
MATH_T(vec3_len, pyda_sqrt(a * a + b * b + c * c))
MATH_QUAD(vec3_dist_sq, (a - b) * (a - b) + (c - d) * (c - d))
MATH_QUAD(vec3_dist, pyda_sqrt((a - b) * (a - b) + (c - d) * (c - d)))
MATH_T(vec3_normalize_x, a / (pyda_sqrt(a*a + b*b + c*c) + 1e-9))
MATH_T(vec3_normalize_y, b / (pyda_sqrt(a*a + b*b + c*c) + 1e-9))
MATH_T(vec3_normalize_z, c / (pyda_sqrt(a*a + b*b + c*c) + 1e-9))
MATH_QUAD(vec3_triple_product, a * b * c)

MATH_QUAD(vec4_dot, a * b + c * d)
MATH_QUAD(vec4_len_sq, a * a + b * b + c * c + d * d)
MATH_QUAD(vec4_len, pyda_sqrt(a * a + b * b + c * c + d * d))
MATH_OCT(vec4_dist_sq, (a - e)*(a - e) + (b - f)*(b - f) + (c - g)*(c - g) + (d - h)*(d - h))
MATH_OCT(vec4_dist, pyda_sqrt((a - e)*(a - e) + (b - f)*(b - f) + (c - g)*(c - g) + (d - h)*(d - h)))
MATH_QUAD(vec4_normalize_x, a / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(vec4_normalize_y, b / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(vec4_normalize_z, c / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(vec4_normalize_w, d / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))

MATH_QUAD(mat2_det, a * d - b * c)
MATH_B(mat2_trace, a + b)
MATH_QUAD(mat2_inverse_scale_00, d / (a * d - b * c + 1e-9))
MATH_QUAD(mat2_inverse_scale_01, -b / (a * d - b * c + 1e-9))
MATH_QUAD(mat2_inverse_scale_10, -c / (a * d - b * c + 1e-9))
MATH_QUAD(mat2_inverse_scale_11, a / (a * d - b * c + 1e-9))
MATH_QUAD(mat2_mul_scalar, a * b * c * d)
MATH_QUAD(mat2_transform_x, a * c + b * d)
MATH_QUAD(mat2_transform_y, c * c + d * d)

MATH_HEX(mat3_det_elem0, a * (e * i - f * h))
MATH_HEX(mat3_det_elem1, b * (d * i - f * g))
MATH_HEX(mat3_det_elem2, c * (d * h - e * g))
MATH_HEX(mat3_trace, a + e + i)
MATH_HEX(mat3_quad_form, a * b * c * d * e * f)
MATH_HEX(mat3_transform_x, a * d + b * e + c * f)
MATH_HEX(mat3_transform_y, d * d + e * e + f * f)
MATH_HEX(mat3_transform_z, a * a + b * b + c * c)

MATH_OCT(mat4_det_approx, a * f * k * p - b * e * l * o)
MATH_OCT(mat4_trace, a + f + k + p)
MATH_OCT(mat4_diag_prod, a * f * k * p)
MATH_OCT(mat4_offdiag_sum, b + c + d + e)

MATH_QUAD(quat_mul_scalar, a * b * c * d)
MATH_QUAD(quat_dot, a * c + b * d)
MATH_QUAD(quat_len_sq, a * a + b * b + c * c + d * d)
MATH_QUAD(quat_len, pyda_sqrt(a * a + b * b + c * c + d * d))
MATH_QUAD(quat_normalize_x, a / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(quat_normalize_y, b / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(quat_normalize_z, c / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(quat_normalize_w, d / (pyda_sqrt(a*a+b*b+c*c+d*d) + 1e-9))
MATH_QUAD(quat_conjugate_x, -a)
MATH_QUAD(quat_conjugate_y, -b)
MATH_QUAD(quat_conjugate_z, -c)
MATH_QUAD(quat_conjugate_w, d)
MATH_QUAD(quat_inverse_x, -a / (a*a+b*b+c*c+d*d + 1e-9))
MATH_QUAD(quat_inverse_y, -b / (a*a+b*b+c*c+d*d + 1e-9))
MATH_QUAD(quat_inverse_z, -c / (a*a+b*b+c*c+d*d + 1e-9))
MATH_QUAD(quat_inverse_w, d / (a*a+b*b+c*c+d*d + 1e-9))

MATH_T(plane_dist_origin, a * a + b * b + c * c)
MATH_QUAD(plane_point_side, a * c + b * d)
MATH_T(ray_plane_intersect, a / (b + 1e-9))
MATH_QUAD(sphere_box_collision_proxy, (a - c) * (a - c) + (b - d) * (b - d))
MATH_T(aabb_overlap_proxy, a < b ? (b < c ? 1.0 : 0.0) : 0.0)
MATH_T(barycentric_u, a * (1.0 - b - c))
MATH_T(barycentric_v, b * (1.0 - a - c))
MATH_T(barycentric_w, c * (1.0 - a - b))
MATH_T(triangle_area_2d, 0.5 * pyda_abs(a * (b - c) + b * (c - a) + c * (a - b)))
MATH_QUAD(triangle_area_3d_approx, 0.5 * pyda_sqrt((a-c)*(a-c) + (b-d)*(b-d)))
MATH_U(gamma_correct_srgb, pyda_pow(x, 1.0 / 2.2))
MATH_U(gamma_correct_linear, pyda_pow(x, 2.2))
MATH_T(color_luminance, a * 0.2126 + b * 0.7152 + c * 0.0722)
MATH_T(color_hue_proxy, (a - b) / (c + 1e-9))
MATH_T(color_saturation_proxy, a / (b + 1e-9))

#endif
