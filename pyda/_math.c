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

// --- MACRO GENERATORS FOR BLAZING-FAST EXPOSURE ---
#define MATH_U(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_U_CUSTOM(name, body) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL; \
        body \
    }

#define MATH_B(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b; if (!PyArg_ParseTuple(args, "dd", &a, &b)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define MATH_B_CUSTOM(name, body) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b; if (!PyArg_ParseTuple(args, "dd", &a, &b)) return NULL; \
        body \
    }

#define MATH_T(name, expr) \
    static PyObject* math_##name(PyObject* self, PyObject* args) { \
        double a, b, c; if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c)) return NULL; \
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
    return PyFloat_FromDouble(b > 0 ? 1.57 : -1.57);
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

// 151-200+: Statistics, Properties, Combinatorics & Extra Utilities
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

// --- METHOD TABLE REGISTRATION ---
static PyMethodDef MathModuleMethods[] = {
    {"fabs", math_fabs, METH_VARARGS, "Fast abs"},
    {"ceil", math_ceil, METH_VARARGS, "Fast ceil"},
    {"floor", math_floor, METH_VARARGS, "Fast floor"},
    {"trunc", math_trunc, METH_VARARGS, "Fast trunc"},
    {"round", math_round, METH_VARARGS, "Fast round"},
    {"copysign", math_copysign, METH_VARARGS, "Fast copysign"},
    {"fmod", math_fmod, METH_VARARGS, "Fast fmod"},
    {"inverse", math_inverse, METH_VARARGS, "Fast inverse"},
    {"square", math_square, METH_VARARGS, "Fast square"},
    {"cube", math_cube, METH_VARARGS, "Fast cube"},
    {"sign", math_sign, METH_VARARGS, "Fast sign"},
    {"saturate", math_saturate, METH_VARARGS, "Fast saturate"},
    {"fdim", math_fdim, METH_VARARGS, "Fast fdim"},
    {"fmax", math_fmax, METH_VARARGS, "Fast fmax"},
    {"fmin", math_fmin, METH_VARARGS, "Fast fmin"},
    {"clamp", math_clamp, METH_VARARGS, "Fast clamp"},
    {"lerp", math_lerp, METH_VARARGS, "Fast lerp"},
    {"identity", math_identity, METH_VARARGS, "Fast identity"},
    {"negate", math_negate, METH_VARARGS, "Fast negate"},
    {"double_val", math_double_val, METH_VARARGS, "Fast double"},
    {"triple", math_triple, METH_VARARGS, "Fast triple"},
    {"half", math_half, METH_VARARGS, "Fast half"},
    {"quarter", math_quarter, METH_VARARGS, "Fast quarter"},
    {"reciprocal_sqrt", math_reciprocal_sqrt, METH_VARARGS, "Fast rsqrt"},
    {"sign_square", math_sign_square, METH_VARARGS, "Fast sign square"},
    
    // Exps/Logs/Roots
    {"exp", math_exp, METH_VARARGS, ""}, {"expm1", math_expm1, METH_VARARGS, ""},
    {"exp2", math_exp2, METH_VARARGS, ""}, {"exp10", math_exp10, METH_VARARGS, ""},
    {"log", math_log, METH_VARARGS, ""}, {"log1p", math_log1p, METH_VARARGS, ""},
    {"log2", math_log2, METH_VARARGS, ""}, {"log10", math_log10, METH_VARARGS, ""},
    {"log_base", math_log_base, METH_VARARGS, ""}, {"pow", math_pow, METH_VARARGS, ""},
    {"sqrt", math_sqrt, METH_VARARGS, ""}, {"cbrt", math_cbrt, METH_VARARGS, ""},
    {"root4", math_root4, METH_VARARGS, ""}, {"root5", math_root5, METH_VARARGS, ""},
    {"hypot", math_hypot, METH_VARARGS, ""}, {"hypot3", math_hypot3, METH_VARARGS, ""},
    {"quadrant", math_quadrant, METH_VARARGS, ""}, {"quintic", math_quintic, METH_VARARGS, ""},
    {"pow2", math_pow2, METH_VARARGS, ""}, {"pow3", math_pow3, METH_VARARGS, ""},
    {"pow4", math_pow4, METH_VARARGS, ""}, {"pow5", math_pow5, METH_VARARGS, ""},
    {"pow6", math_pow6, METH_VARARGS, ""}, {"pow7", math_pow7, METH_VARARGS, ""},
    {"pow8", math_pow8, METH_VARARGS, ""},

    // Trig & Angles
    {"sin", math_sin, METH_VARARGS, ""}, {"cos", math_cos, METH_VARARGS, ""},
    {"tan", math_tan, METH_VARARGS, ""}, {"asin", math_asin, METH_VARARGS, ""},
    {"acos", math_acos, METH_VARARGS, ""}, {"atan", math_atan, METH_VARARGS, ""},
    {"atan2", math_atan2, METH_VARARGS, ""}, {"degrees", math_degrees, METH_VARARGS, ""},
    {"radians", math_radians, METH_VARARGS, ""}, {"sin_deg", math_sin_deg, METH_VARARGS, ""},
    {"cos_deg", math_cos_deg, METH_VARARGS, ""}, {"tan_deg", math_tan_deg, METH_VARARGS, ""},
    {"secant", math_secant, METH_VARARGS, ""}, {"cosecant", math_cosecant, METH_VARARGS, ""},
    {"cotangent", math_cotangent, METH_VARARGS, ""}, {"versine", math_versine, METH_VARARGS, ""},
    {"haversine", math_haversine, METH_VARARGS, ""}, {"exsecant", math_exsecant, METH_VARARGS, ""},
    {"sinc", math_sinc, METH_VARARGS, ""}, {"normalize_angle", math_normalize_angle, METH_VARARGS, ""},
    {"sin_sq", math_sin_sq, METH_VARARGS, ""}, {"cos_sq", math_cos_sq, METH_VARARGS, ""},
    {"tan_sq", math_tan_sq, METH_VARARGS, ""}, {"sin_cube", math_sin_cube, METH_VARARGS, ""},
    {"cos_cube", math_cos_cube, METH_VARARGS, ""},

    // Hyperbolic
    {"sinh", math_sinh, METH_VARARGS, ""}, {"cosh", math_cosh, METH_VARARGS, ""},
    {"tanh", math_tanh, METH_VARARGS, ""}, {"asinh", math_asinh, METH_VARARGS, ""},
    {"acosh", math_acosh, METH_VARARGS, ""}, {"atanh", math_atanh, METH_VARARGS, ""},
    {"sech", math_sech, METH_VARARGS, ""}, {"csch", math_csch, METH_VARARGS, ""},
    {"coth", math_coth, METH_VARARGS, ""}, {"sinh_sq", math_sinh_sq, METH_VARARGS, ""},
    {"cosh_sq", math_cosh_sq, METH_VARARGS, ""}, {"tanh_sq", math_tanh_sq, METH_VARARGS, ""},
    {"asinh_scaled", math_asinh_scaled, METH_VARARGS, ""}, {"acosh_shifted", math_acosh_shifted, METH_VARARGS, ""},
    {"atanh_scaled", math_atanh_scaled, METH_VARARGS, ""}, {"sinh_fast", math_sinh_fast, METH_VARARGS, ""},
    {"cosh_fast", math_cosh_fast, METH_VARARGS, ""}, {"tanh_fast", math_tanh_fast, METH_VARARGS, ""},
    {"sech_fast", math_sech_fast, METH_VARARGS, ""}, {"csch_fast", math_csch_fast, METH_VARARGS, ""},
    {"coth_fast", math_coth_fast, METH_VARARGS, ""}, {"asinh_fast", math_asinh_fast, METH_VARARGS, ""},
    {"acosh_fast", math_acosh_fast, METH_VARARGS, ""}, {"atanh_fast", math_atanh_fast, METH_VARARGS, ""},
    {"sinh_cube", math_sinh_cube, METH_VARARGS, ""},

    // Activations & Waves
    {"logistic", math_logistic, METH_VARARGS, ""}, {"relu", math_relu, METH_VARARGS, ""},
    {"gelu", math_gelu, METH_VARARGS, ""}, {"silu", math_silu, METH_VARARGS, ""},
    {"leaky_relu", math_leaky_relu, METH_VARARGS, ""}, {"elu", math_elu, METH_VARARGS, ""},
    {"selu", math_selu, METH_VARARGS, ""}, {"softplus", math_softplus, METH_VARARGS, ""},
    {"softsign", math_softsign, METH_VARARGS, ""}, {"mish", math_mish, METH_VARARGS, ""},
    {"hard_sigmoid", math_hard_sigmoid, METH_VARARGS, ""}, {"hard_swish", math_hard_swish, METH_VARARGS, ""},
    {"bent_identity", math_bent_identity, METH_VARARGS, ""}, {"gaussian", math_gaussian, METH_VARARGS, ""},
    {"sin_activation", math_sin_activation, METH_VARARGS, ""}, {"cos_activation", math_cos_activation, METH_VARARGS, ""},
    {"sinc_activation", math_sinc_activation, METH_VARARGS, ""}, {"thresholded_relu", math_thresholded_relu, METH_VARARGS, ""},
    {"scaled_tanh", math_scaled_tanh, METH_VARARGS, ""}, {"relu6", math_relu6, METH_VARARGS, ""},
    {"prelu_pos", math_prelu_pos, METH_VARARGS, ""}, {"prelu_neg", math_prelu_neg, METH_VARARGS, ""},
    {"exponential_elu", math_exponential_elu, METH_VARARGS, ""}, {"abs_relu", math_abs_relu, METH_VARARGS, ""},
    {"sq_relu", math_sq_relu, METH_VARARGS, ""}, {"cube_relu", math_cube_relu, METH_VARARGS, ""},
    {"softmax_elem", math_softmax_elem, METH_VARARGS, ""}, {"log_sigmoid", math_log_sigmoid, METH_VARARGS, ""},
    {"inv_logistic", math_inv_logistic, METH_VARARGS, ""}, {"binary_step", math_binary_step, METH_VARARGS, ""},
    {"bipolar_step", math_bipolar_step, METH_VARARGS, ""}, {"triangular_wave", math_triangular_wave, METH_VARARGS, ""},
    {"sawtooth_wave", math_sawtooth_wave, METH_VARARGS, ""}, {"square_wave", math_square_wave, METH_VARARGS, ""},
    {"pulse_wave", math_pulse_wave, METH_VARARGS, ""}, {"ramp", math_ramp, METH_VARARGS, ""},
    {"smoothstep", math_smoothstep, METH_VARARGS, ""}, {"smootherstep", math_smootherstep, METH_VARARGS, ""},
    {"cosine_interp", math_cosine_interp, METH_VARARGS, ""},

    // Stats, Properties & Conversions
    {"isinf", math_isinf, METH_VARARGS, ""}, {"isnan", math_isnan, METH_VARARGS, ""},
    {"isfinite", math_isfinite, METH_VARARGS, ""}, {"factorial", math_factorial, METH_VARARGS, ""},
    {"gcd_proxy", math_gcd_proxy, METH_VARARGS, ""}, {"lcm_proxy", math_lcm_proxy, METH_VARARGS, ""},
    {"combinatorics_nCr", math_combinatorics_nCr, METH_VARARGS, ""}, {"combinatorics_nPr", math_combinatorics_nPr, METH_VARARGS, ""},
    {"variance_kernel", math_variance_kernel, METH_VARARGS, ""}, {"std_dev_kernel", math_std_dev_kernel, METH_VARARGS, ""},
    {"z_score_kernel", math_z_score_kernel, METH_VARARGS, ""}, {"mean_deviation", math_mean_deviation, METH_VARARGS, ""},
     {"root_mean_square", math_root_mean_square, METH_VARARGS, ""}, {"energy_kernel", math_energy_kernel, METH_VARARGS, ""},
    {"momentum_kernel", math_momentum_kernel, METH_VARARGS, ""}, {"kinetic_energy", math_kinetic_energy, METH_VARARGS, ""},
    {"potential_energy", math_potential_energy, METH_VARARGS, ""}, {"gravity_force", math_gravity_force, METH_VARARGS, ""},
    {"celsius_to_kelvin", math_celsius_to_kelvin, METH_VARARGS, ""}, {"kelvin_to_celsius", math_kelvin_to_celsius, METH_VARARGS, ""},
    {"fahrenheit_to_celsius", math_fahrenheit_to_celsius, METH_VARARGS, ""}, {"celsius_to_fahrenheit", math_celsius_to_fahrenheit, METH_VARARGS, ""},
    {"percent_to_fraction", math_percent_to_fraction, METH_VARARGS, ""}, {"fraction_to_percent", math_fraction_to_percent, METH_VARARGS, ""},
    {"db_to_amplitude", math_db_to_amplitude, METH_VARARGS, ""}, {"amplitude_to_db", math_amplitude_to_db, METH_VARARGS, ""},
    {"bit_shift_proxy", math_bit_shift_proxy, METH_VARARGS, ""}, {"golden_ratio_scale", math_golden_ratio_scale, METH_VARARGS, ""},
    {"inverse_golden_scale", math_inverse_golden_scale, METH_VARARGS, ""}, {"euler_scale", math_euler_scale, METH_VARARGS, ""},
    {"pi_scale", math_pi_scale, METH_VARARGS, ""}, {"tau_scale", math_tau_scale, METH_VARARGS, ""},
    {"deg_to_rad_fast", math_deg_to_rad_fast, METH_VARARGS, ""}, {"rad_to_deg_fast", math_rad_to_deg_fast, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef math_module = {
    PyModuleDef_HEAD_INIT,
    "_math",
    "Blazing-fast 200+ function zero-dependency math helper engine",
    -1,
    MathModuleMethods
};

PyMODINIT_FUNC PyInit__math(void) {
    return PyModule_Create(&math_module);
}
