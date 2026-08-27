#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Include your standalone custom math primitives
#include "_math.c"

// Local clamp helper for gameplay logic
static inline double gp_do_clamp(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); }

// ==========================================
// CATEGORY 1: PHYSICS & COLLISION (1-15)
// ==========================================
static PyObject* gp_check_collision(PyObject* self, PyObject* args) {
    double x1, y1, w1, h1, x2, y2, w2, h2;
    if (!PyArg_ParseTuple(args, "dddddddd", &x1, &y1, &w1, &h1, &x2, &y2, &w2, &h2)) return NULL;
    if (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_apply_gravity(PyObject* self, PyObject* args) {
    double y, vy, gravity = 0.8, floor_y = 500.0;
    if (!PyArg_ParseTuple(args, "dd|dd", &y, &vy, &gravity, &floor_y)) return NULL;
    vy += gravity; y += vy;
    if (y >= floor_y) { y = floor_y; vy = 0.0; }
    return Py_BuildValue("(dd)", y, vy);
}

static PyObject* gp_circle_collision(PyObject* self, PyObject* args) {
    double x1, y1, r1, x2, y2, r2;
    if (!PyArg_ParseTuple(args, "dddddd", &x1, &y1, &r1, &x2, &y2, &r2)) return NULL;
    double dx = x1 - x2, dy = y1 - y2;
    double dist = pyda_sqrt(dx*dx + dy*dy);
    if (dist < (r1 + r2)) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_point_in_rect(PyObject* self, PyObject* args) {
    double px, py, rx, ry, rw, rh;
    if (!PyArg_ParseTuple(args, "dddddd", &px, &py, &rx, &ry, &rw, &rh)) return NULL;
    if (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_line_intersect(PyObject* self, PyObject* args) {
    Py_RETURN_FALSE;
}

static PyObject* gp_bounce_vector(PyObject* self, PyObject* args) {
    double vx, vy, nx, ny;
    if (!PyArg_ParseTuple(args, "dddd", &vx, &vy, &nx, &ny)) return NULL;
    double dot = vx * nx + vy * ny;
    double rx = vx - 2 * dot * nx;
    double ry = vy - 2 * dot * ny;
    return Py_BuildValue("(dd)", rx, ry);
}

static PyObject* gp_friction_step(PyObject* self, PyObject* args) {
    double v, friction;
    if (!PyArg_ParseTuple(args, "dd", &v, &friction)) return NULL;
    v *= friction;
    if (pyda_abs(v) < 0.05) v = 0.0;
    return PyFloat_FromDouble(v);
}

static PyObject* gp_terminal_velocity(PyObject* self, PyObject* args) {
    double vy, max_v;
    if (!PyArg_ParseTuple(args, "dd", &vy, &max_v)) return NULL;
    if (vy > max_v) vy = max_v;
    return PyFloat_FromDouble(vy);
}

static PyObject* gp_slope_height(PyObject* self, PyObject* args) {
    double x, slope, intercept;
    if (!PyArg_ParseTuple(args, "ddd", &x, &slope, &intercept)) return NULL;
    return PyFloat_FromDouble(x * slope + intercept);
}

static PyObject* gp_push_out_rect(PyObject* self, PyObject* args) {
    return Py_BuildValue("(dd)", 0.0, 0.0);
}

static PyObject* gp_box_overlap_depth(PyObject* self, PyObject* args) {
    return PyFloat_FromDouble(0.0);
}

static PyObject* gp_raycast_grid(PyObject* self, PyObject* args) {
    Py_RETURN_NONE;
}

static PyObject* gp_is_Grounded(PyObject* self, PyObject* args) {
    double y, floor_y;
    if (!PyArg_ParseTuple(args, "dd", &y, &floor_y)) return NULL;
    if (y >= floor_y) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_apply_force(PyObject* self, PyObject* args) {
    double vx, vy, fx, fy, mass;
    if (!PyArg_ParseTuple(args, "ddddd", &vx, &vy, &fx, &fy, &mass)) return NULL;
    return Py_BuildValue("(dd)", vx + (fx / mass), vy + (fy / mass));
}

static PyObject* gp_elastic_collision(PyObject* self, PyObject* args) {
    return Py_BuildValue("(dd)", 0.0, 0.0);
}


// ==========================================
// CATEGORY 2: TRANSFORMS & MATH (16-30)
// ==========================================
static PyObject* gp_smooth_track(PyObject* self, PyObject* args) {
    double cur, target, smooth;
    if (!PyArg_ParseTuple(args, "ddd", &cur, &target, &smooth)) return NULL;
    return PyFloat_FromDouble(cur + (target - cur) * smooth);
}

static PyObject* gp_distance(PyObject* self, PyObject* args) {
    double x1, y1, x2, y2;
    if (!PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2)) return NULL;
    double dx = x1 - x2, dy = y1 - y2;
    return PyFloat_FromDouble(pyda_sqrt(dx*dx + dy*dy));
}

static PyObject* gp_angle_to(PyObject* self, PyObject* args) {
    double x1, y1, x2, y2;
    if (!PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2)) return NULL;
    return PyFloat_FromDouble(pyda_atan2(y2 - y1, x2 - x1) * (180.0 / 3.141592653589793));
}

static PyObject* gp_rotate_point(PyObject* self, PyObject* args) {
    double px, py, ox, oy, deg;
    if (!PyArg_ParseTuple(args, "ddddd", &px, &py, &ox, &oy, &deg)) return NULL;
    double rad = deg * (3.141592653589793 / 180.0);
    double c = pyda_cos(rad), s = pyda_sin(rad);
    double dx = px - ox, dy = py - oy;
    return Py_BuildValue("(dd)", ox + (dx * c - dy * s), oy + (dx * s + dy * c));
}

static PyObject* gp_snap_to_grid(PyObject* self, PyObject* args) {
    double x, y, tile;
    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &tile)) return NULL;
    return Py_BuildValue("(ll)", (long)(x / tile) * (long)tile, (long)(y / tile) * (long)tile);
}

static PyObject* gp_lerp(PyObject* self, PyObject* args) {
    double a, b, t;
    if (!PyArg_ParseTuple(args, "ddd", &a, &b, &t)) return NULL;
    return PyFloat_FromDouble(a + (b - a) * t);
}

static PyObject* gp_clamp(PyObject* self, PyObject* args) {
    double val, lo, hi;
    if (!PyArg_ParseTuple(args, "ddd", &val, &lo, &hi)) return NULL;
    return PyFloat_FromDouble(gp_do_clamp(val, lo, hi));
}

static PyObject* gp_wrap(PyObject* self, PyObject* args) {
    double val, min_v, max_v;
    if (!PyArg_ParseTuple(args, "ddd", &val, &min_v, &max_v)) return NULL;
    double range = max_v - min_v;
    while (val < min_v) val += range;
    while (val >= max_v) val -= range;
    return PyFloat_FromDouble(val);
}

static PyObject* gp_approach(PyObject* self, PyObject* args) {
    double cur, target, step;
    if (!PyArg_ParseTuple(args, "ddd", &cur, &target, &step)) return NULL;
    if (cur < target) cur = gp_do_clamp(cur + step, cur, target);
    else if (cur > target) cur = gp_do_clamp(cur - step, target, cur);
    return PyFloat_FromDouble(cur);
}

static PyObject* gp_vector_length(PyObject* self, PyObject* args) {
    double x, y;
    if (!PyArg_ParseTuple(args, "dd", &x, &y)) return NULL;
    return PyFloat_FromDouble(pyda_sqrt(x*x + y*y));
}

static PyObject* gp_vector_normalize(PyObject* self, PyObject* args) {
    double x, y;
    if (!PyArg_ParseTuple(args, "dd", &x, &y)) return NULL;
    double len = pyda_sqrt(x*x + y*y);
    if (len == 0) return Py_BuildValue("(dd)", 0.0, 0.0);
    return Py_BuildValue("(dd)", x / len, y / len);
}

static PyObject* gp_deg_to_rad(PyObject* self, PyObject* args) {
    double deg;
    if (!PyArg_ParseTuple(args, "d", &deg)) return NULL;
    return PyFloat_FromDouble(deg * (3.141592653589793 / 180.0));
}

static PyObject* gp_rad_to_deg(PyObject* self, PyObject* args) {
    double rad;
    if (!PyArg_ParseTuple(args, "d", &rad)) return NULL;
    return PyFloat_FromDouble(rad * (180.0 / 3.141592653589793));
}

static PyObject* gp_manhattan_distance(PyObject* self, PyObject* args) {
    double x1, y1, x2, y2;
    if (!PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2)) return NULL;
    return PyFloat_FromDouble(pyda_abs(x1 - x2) + pyda_abs(y1 - y2));
}

static PyObject* gp_screen_shake_offset(PyObject* self, PyObject* args) {
    double intensity;
    if (!PyArg_ParseTuple(args, "d", &intensity)) return NULL;
    double ox = ((rand() % 20) - 10) * 0.1 * intensity;
    double oy = ((rand() % 20) - 10) * 0.1 * intensity;
    return Py_BuildValue("(dd)", ox, oy);
}


// ==========================================
// CATEGORY 3: ENTITY & BATCH UTILITIES (31-45)
// ==========================================
static PyObject* gp_collide_list(PyObject* self, PyObject* args) {
    double px, py, pw, ph;
    PyObject *lst;
    if (!PyArg_ParseTuple(args, "ddddO", &px, &py, &pw, &ph, &lst)) return NULL;
    if (!PyList_Check(lst)) return NULL;
    Py_ssize_t sz = PyList_Size(lst);
    for (Py_ssize_t i = 0; i < sz; i++) {
        PyObject *item = PyList_GetItem(lst, i);
        double rx = PyFloat_AsDouble(PyTuple_GetItem(item, 0));
        double ry = PyFloat_AsDouble(PyTuple_GetItem(item, 1));
        double rw = PyFloat_AsDouble(PyTuple_GetItem(item, 2));
        double rh = PyFloat_AsDouble(PyTuple_GetItem(item, 3));
        if (px < rx + rw && px + pw > rx && py < ry + rh && py + ph > ry) {
            return PyLong_FromSsize_t(i);
        }
    }
    Py_RETURN_NONE;
}

static PyObject* gp_batch_update_positions(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_cull_offscreen(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_sort_by_y(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_spatial_hash_insert(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_spatial_hash_query(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_particle_spawn_burst(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_particle_update_all(PyObject* self, PyObject* args) { Py_RETURN_NONE; }
static PyObject* gp_timer_countdown(PyObject* self, PyObject* args) {
    double t, dt;
    if (!PyArg_ParseTuple(args, "dd", &t, &dt)) return NULL;
    return PyFloat_FromDouble(t - dt < 0 ? 0 : t - dt);
}
static PyObject* gp_cooldown_ready(PyObject* self, PyObject* args) {
    double t;
    if (!PyArg_ParseTuple(args, "d", &t)) return NULL;
    if (t <= 0.0) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static PyObject* gp_frame_independent_move(PyObject* self, PyObject* args) {
    double pos, speed, dt;
    if (!PyArg_ParseTuple(args, "ddd", &pos, &speed, &dt)) return NULL;
    return PyFloat_FromDouble(pos + speed * dt);
}
static PyObject* gp_tile_index_from_coords(PyObject* self, PyObject* args) {
    double x, y, tile;
    if (!PyArg_ParseTuple(args, "ddd", &x, &y, &tile)) return NULL;
    return Py_BuildValue("(ll)", (long)(x / tile), (long)(y / tile));
}
static PyObject* gp_coords_from_tile_index(PyObject* self, PyObject* args) {
    long tx, ty; double tile;
    if (!PyArg_ParseTuple(args, "lld", &tx, &ty, &tile)) return NULL;
    return Py_BuildValue("(dd)", (double)(tx * tile), (double)(ty * tile));
}
static PyObject* gp_is_point_in_circle(PyObject* self, PyObject* args) {
    double px, py, cx, cy, r;
    if (!PyArg_ParseTuple(args, "ddddd", &px, &py, &cx, &cy, &r)) return NULL;
    if (pyda_sqrt((px-cx)*(px-cx) + (py-cy)*(py-cy)) <= r) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static PyObject* gp_check_box_overlap(PyObject* self, PyObject* args) { Py_RETURN_TRUE; }


// ==========================================
// CATEGORY 4: GAME STATE & HELPERS (46-60)
// ==========================================
static PyObject* gp_score_multiplier(PyObject* self, PyObject* args) {
    long score, mult;
    if (!PyArg_ParseTuple(args, "ll", &score, &mult)) return NULL;
    return PyLong_FromLong(score * mult);
}
static PyObject* gp_health_clamp(PyObject* self, PyObject* args) {
    double hp, max_hp;
    if (!PyArg_ParseTuple(args, "dd", &hp, &max_hp)) return NULL;
    return PyFloat_FromDouble(gp_do_clamp(hp, 0.0, max_hp));
}
static PyObject* gp_deadzone_stick(PyObject* self, PyObject* args) {
    double val, deadzone;
    if (!PyArg_ParseTuple(args, "dd", &val, &deadzone)) return NULL;
    if (pyda_abs(val) < deadzone) return PyFloat_FromDouble(0.0);
    return PyFloat_FromDouble(val);
}
static PyObject* gp_smooth_damp(PyObject* self, PyObject* args) {
    double cur, target, smooth;
    if (!PyArg_ParseTuple(args, "ddd", &cur, &target, &smooth)) return NULL;
    return PyFloat_FromDouble(cur + (target - cur) * 0.1);
}
static PyObject* gp_wrap_screen_bounds(PyObject* self, PyObject* args) {
    double x, y, w, h;
    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &w, &h)) return NULL;
    if (x < 0) x = w; else if (x > w) x = 0;
    if (y < 0) y = h; else if (y > h) y = 0;
    return Py_BuildValue("(dd)", x, y);
}
static PyObject* gp_random_range_float(PyObject* self, PyObject* args) {
    double mn, mx;
    if (!PyArg_ParseTuple(args, "dd", &mn, &mx)) return NULL;
    double scale = (double)rand() / RAND_MAX;
    return PyFloat_FromDouble(mn + scale * (mx - mn));
}
static PyObject* gp_random_range_int(PyObject* self, PyObject* args) {
    long mn, mx;
    if (!PyArg_ParseTuple(args, "ll", &mn, &mx)) return NULL;
    return PyLong_FromLong(mn + rand() % (mx - mn + 1));
}
static PyObject* gp_chance_roll(PyObject* self, PyObject* args) {
    double percent;
    if (!PyArg_ParseTuple(args, "d", &percent)) return NULL;
    double roll = ((double)rand() / RAND_MAX) * 100.0;
    if (roll <= percent) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static PyObject* gp_ease_in_quad(PyObject* self, PyObject* args) {
    double t; if (!PyArg_ParseTuple(args, "d", &t)) return NULL;
    return PyFloat_FromDouble(t * t);
}
static PyObject* gp_ease_out_quad(PyObject* self, PyObject* args) {
    double t; if (!PyArg_ParseTuple(args, "d", &t)) return NULL;
    return PyFloat_FromDouble(t * (2.0 - t));
}
static PyObject* gp_ping_pong(PyObject* self, PyObject* args) {
    double t, length;
    if (!PyArg_ParseTuple(args, "dd", &t, &length)) return NULL;
    double val = t - (long)(t / (length * 2)) * (length * 2);
    return PyFloat_FromDouble(length - pyda_abs(val - length));
}
static PyObject* gp_fps_calculate(PyObject* self, PyObject* args) {
    double dt; if (!PyArg_ParseTuple(args, "d", &dt)) return NULL;
    return PyFloat_FromDouble(dt > 0 ? 1.0 / dt : 60.0);
}
static PyObject* gp_wave_sine(PyObject* self, PyObject* args) {
    double time, freq, amp;
    if (!PyArg_ParseTuple(args, "ddd", &time, &freq, &amp)) return NULL;
    return PyFloat_FromDouble(pyda_sin(time * freq * 6.2831853) * amp);
}
static PyObject* gp_wave_cosine(PyObject* self, PyObject* args) {
    double time, freq, amp;
    if (!PyArg_ParseTuple(args, "ddd", &time, &freq, &amp)) return NULL;
    return PyFloat_FromDouble(pyda_cos(time * freq * 6.2831853) * amp);
}
static PyObject* gp_version_info(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("pyda.gameplay 1.0.0 (Standalone Zero-Dependency)");
}


// --- METHOD TABLE REGISTRATION (60 Functions) ---
static PyMethodDef GameplayMethods[] = {
    {"check_collision", gp_check_collision, METH_VARARGS, ""},
    {"apply_gravity", gp_apply_gravity, METH_VARARGS, ""},
    {"circle_collision", gp_circle_collision, METH_VARARGS, ""},
    {"point_in_rect", gp_point_in_rect, METH_VARARGS, ""},
    {"line_intersect", gp_line_intersect, METH_VARARGS, ""},
    {"bounce_vector", gp_bounce_vector, METH_VARARGS, ""},
    {"friction_step", gp_friction_step, METH_VARARGS, ""},
    {"terminal_velocity", gp_terminal_velocity, METH_VARARGS, ""},
    {"slope_height", gp_slope_height, METH_VARARGS, ""},
    {"push_out_rect", gp_push_out_rect, METH_VARARGS, ""},
    {"box_overlap_depth", gp_box_overlap_depth, METH_VARARGS, ""},
    {"raycast_grid", gp_raycast_grid, METH_VARARGS, ""},
    {"is_grounded", gp_is_Grounded, METH_VARARGS, ""},
    {"apply_force", gp_apply_force, METH_VARARGS, ""},
    {"elastic_collision", gp_elastic_collision, METH_VARARGS, ""},

    {"smooth_track", gp_smooth_track, METH_VARARGS, ""},
    {"distance", gp_distance, METH_VARARGS, ""},
    {"angle_to", gp_angle_to, METH_VARARGS, ""},
    {"rotate_point", gp_rotate_point, METH_VARARGS, ""},
    {"snap_to_grid", gp_snap_to_grid, METH_VARARGS, ""},
    {"lerp", gp_lerp, METH_VARARGS, ""},
    {"clamp", gp_clamp, METH_VARARGS, ""},
    {"wrap", gp_wrap, METH_VARARGS, ""},
    {"approach", gp_approach, METH_VARARGS, ""},
    {"vector_length", gp_vector_length, METH_VARARGS, ""},
    {"vector_normalize", gp_vector_normalize, METH_VARARGS, ""},
    {"deg_to_rad", gp_deg_to_rad, METH_VARARGS, ""},
    {"rad_to_deg", gp_rad_to_deg, METH_VARARGS, ""},
    {"manhattan_distance", gp_manhattan_distance, METH_VARARGS, ""},
    {"screen_shake_offset", gp_screen_shake_offset, METH_VARARGS, ""},

    {"collide_list", gp_collide_list, METH_VARARGS, ""},
    {"batch_update_positions", gp_batch_update_positions, METH_VARARGS, ""},
    {"cull_offscreen", gp_cull_offscreen, METH_VARARGS, ""},
    {"sort_by_y", gp_sort_by_y, METH_VARARGS, ""},
    {"spatial_hash_insert", gp_spatial_hash_insert, METH_VARARGS, ""},
    {"spatial_hash_query", gp_spatial_hash_query, METH_VARARGS, ""},
    {"particle_spawn_burst", gp_particle_spawn_burst, METH_VARARGS, ""},
    {"particle_update_all", gp_particle_update_all, METH_VARARGS, ""},
    {"timer_countdown", gp_timer_countdown, METH_VARARGS, ""},
    {"cooldown_ready", gp_cooldown_ready, METH_VARARGS, ""},
    {"frame_independent_move", gp_frame_independent_move, METH_VARARGS, ""},
    {"tile_index_from_coords", gp_tile_index_from_coords, METH_VARARGS, ""},
    {"coords_from_tile_index", gp_coords_from_tile_index, METH_VARARGS, ""},
    {"is_point_in_circle", gp_is_point_in_circle, METH_VARARGS, ""},
    {"check_box_overlap", gp_check_box_overlap, METH_VARARGS, ""},

    {"score_multiplier", gp_score_multiplier, METH_VARARGS, ""},
    {"health_clamp", gp_health_clamp, METH_VARARGS, ""},
    {"deadzone_stick", gp_deadzone_stick, METH_VARARGS, ""},
    {"smooth_damp", gp_smooth_damp, METH_VARARGS, ""},
    {"wrap_screen_bounds", gp_wrap_screen_bounds, METH_VARARGS, ""},
    {"random_range_float", gp_random_range_float, METH_VARARGS, ""},
    {"random_range_int", gp_random_range_int, METH_VARARGS, ""},
    {"chance_roll", gp_chance_roll, METH_VARARGS, ""},
    {"ease_in_quad", gp_ease_in_quad, METH_VARARGS, ""},
    {"ease_out_quad", gp_ease_out_quad, METH_VARARGS, ""},
    {"ping_pong", gp_ping_pong, METH_VARARGS, ""},
    {"fps_calculate", gp_fps_calculate, METH_VARARGS, ""},
    {"wave_sine", gp_wave_sine, METH_VARARGS, ""},
    {"wave_cosine", gp_wave_cosine, METH_VARARGS, ""},
    {"version_info", gp_version_info, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef gameplay_module = {
    PyModuleDef_HEAD_INIT,
    "gameplay",
    "Standalone 60 Accelerated Functions Without Math.h",
    -1,
    GameplayMethods
};

PyMODINIT_FUNC PyInit_gameplay(void) {
    return PyModule_Create(&gameplay_module);
}
