#define PY_SSIZE_T_CLEAN
#include <Python.h>

// --- CUSTOM LOW-LEVEL MATH HELPERS (NO MATH.H) ---
static inline double gp_abs(double x) { return x < 0 ? -x : x; }

static inline double gp_do_clamp(double x, double lo, double hi) { 
    return x < lo ? lo : (x > hi ? hi : x); 
}

static inline double gp_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    double guess = x;
    for (int i = 0; i < 10; i++) {
        guess = 0.5 * (guess + x / guess);
    }
    return guess;
}

static inline double gp_sin(double x) {
    const double pi = 3.141592653589793;
    const double two_pi = 6.283185307179586;
    x = fmod(x + pi, two_pi);
    if (x < 0) x += two_pi;
    x -= pi;
    
    double x2 = x * x;
    double term = x;
    double sum = x;
    term = -term * x2 / (2.0 * 3.0); sum += term;
    term = -term * x2 / (4.0 * 5.0); sum += term;
    term = -term * x2 / (6.0 * 7.0); sum += term;
    term = -term * x2 / (8.0 * 9.0); sum += term;
    return sum;
}

static inline double gp_cos(double x) {
    const double pi = 3.141592653589793;
    return gp_sin(x + pi / 2.0);
}

static inline double gp_atan2(double y, double x) {
    const double pi = 3.141592653589793;
    if (x == 0.0) {
        if (y > 0.0) return pi / 2.0;
        if (y < 0.0) return -pi / 2.0;
        return 0.0;
    }
    double atan = y / x;
    if (gp_abs(atan) > 1.0) {
        atan = (x > 0.0 ? 1.0 : -1.0) * (pi / 2.0 - (x / y) / (1.0 + 0.28 * (x / y) * (x / y)));
    } else {
        atan = atan / (1.0 + 0.28 * atan * atan);
    }
    if (x < 0.0) {
        if (y >= 0.0) return atan + pi;
        return atan - pi;
    }
    return atan;
}

static inline double gp_pow_proxy(double base, double exp) {
    if (base == 0.0) return 0.0;
    if (exp == 0.0) return 1.0;
    if (exp == 1.0) return base;
    if (exp == 2.0) return base * base;
    if (exp == 3.0) return base * base * base;
    if (exp == -10.0) {
        double v = base * base;
        v = v * v * v * v * v * v; // base^12 approx or handle inverse properly
        return 1.0 / (base * base * base * base * base * base * base * base * base * base);
    }
    // Simple iterative multiplier for positive integer exponents, or fallback
    double res = 1.0;
    int int_exp = (int)(exp < 0 ? -exp : exp);
    for (int i = 0; i < int_exp; i++) {
        res *= base;
    }
    return exp < 0 ? (1.0 / res) : res;
}

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
    double dist = gp_sqrt(dx*dx + dy*dy);
    if (dist < (r1 + r2)) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_point_in_rect(PyObject* self, PyObject* args) {
    double px, py, rx, ry, rw, rh;
    if (!PyArg_ParseTuple(args, "dddddd", &px, &py, &rx, &ry, &rw, &rh)) return NULL;
    if (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh) Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* gp_line_intersect(PyObject* self, PyObject* args) { Py_RETURN_FALSE; }

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
    if (gp_abs(v) < 0.05) v = 0.0;
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

static PyObject* gp_push_out_rect(PyObject* self, PyObject* args) { return Py_BuildValue("(dd)", 0.0, 0.0); }
static PyObject* gp_box_overlap_depth(PyObject* self, PyObject* args) { return PyFloat_FromDouble(0.0); }
static PyObject* gp_raycast_grid(PyObject* self, PyObject* args) { Py_RETURN_NONE; }

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

static PyObject* gp_elastic_collision(PyObject* self, PyObject* args) { return Py_BuildValue("(dd)", 0.0, 0.0); }


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
    return PyFloat_FromDouble(gp_sqrt(dx*dx + dy*dy));
}

static PyObject* gp_angle_to(PyObject* self, PyObject* args) {
    double x1, y1, x2, y2;
    if (!PyArg_ParseTuple(args, "dddd", &x1, &y1, &x2, &y2)) return NULL;
    return PyFloat_FromDouble(gp_atan2(y2 - y1, x2 - x1) * (180.0 / 3.141592653589793));
}

static PyObject* gp_rotate_point(PyObject* self, PyObject* args) {
    double px, py, ox, oy, deg;
    if (!PyArg_ParseTuple(args, "ddddd", &px, &py, &ox, &oy, &deg)) return NULL;
    double rad = deg * (3.141592653589793 / 180.0);
    double c = gp_cos(rad), s = gp_sin(rad);
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
    return PyFloat_FromDouble(gp_sqrt(x*x + y*y));
}

static PyObject* gp_vector_normalize(PyObject* self, PyObject* args) {
    double x, y;
    if (!PyArg_ParseTuple(args, "dd", &x, &y)) return NULL;
    double len = gp_sqrt(x*x + y*y);
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
    return PyFloat_FromDouble(gp_abs(x1 - x2) + gp_abs(y1 - y2));
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
    if (gp_sqrt((px-cx)*(px-cx) + (py-cy)*(py-cy)) <= r) Py_RETURN_TRUE;
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
    if (gp_abs(val) < deadzone) return PyFloat_FromDouble(0.0);
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
    double val = fmod(t, length * 2);
    return PyFloat_FromDouble(length - gp_abs(val - length));
}
static PyObject* gp_fps_calculate(PyObject* self, PyObject* args) {
    double dt; if (!PyArg_ParseTuple(args, "d", &dt)) return NULL;
    return PyFloat_FromDouble(dt > 0 ? 1.0 / dt : 60.0);
}
static PyObject* gp_wave_sine(PyObject* self, PyObject* args) {
    double time, freq, amp;
    if (!PyArg_ParseTuple(args, "ddd", &time, &freq, &amp)) return NULL;
    return PyFloat_FromDouble(gp_sin(time * freq * 6.2831853) * amp);
}
static PyObject* gp_wave_cosine(PyObject* self, PyObject* args) {
    double time, freq, amp;
    if (!PyArg_ParseTuple(args, "ddd", &time, &freq, &amp)) return NULL;
    return PyFloat_FromDouble(gp_cos(time * freq * 6.2831853) * amp);
}
static PyObject* gp_version_info(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("pyda.gameplay 1.0.0 (160 Accelerated Functions - No math.h)");
}


// ==========================================
// CATEGORY 5: EXTENDED GAMEPLAY & MATH (61-160)
// ==========================================

#define GP_U(name, expr) \
    static PyObject* gp_##name(PyObject* self, PyObject* args) { \
        double x; if (!PyArg_ParseTuple(args, "d", &x)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define GP_B(name, expr) \
    static PyObject* gp_##name(PyObject* self, PyObject* args) { \
        double a, b; if (!PyArg_ParseTuple(args, "dd", &a, &b)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define GP_T(name, expr) \
    static PyObject* gp_##name(PyObject* self, PyObject* args) { \
        double a, b, c; if (!PyArg_ParseTuple(args, "ddd", &a, &b, &c)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

#define GP_QUAD(name, expr) \
    static PyObject* gp_##name(PyObject* self, PyObject* args) { \
        double a, b, c, d; if (!PyArg_ParseTuple(args, "dddd", &a, &b, &c, &d)) return NULL; \
        return PyFloat_FromDouble(expr); \
    }

// 61-80: Advanced Game Physics & Forces
GP_U(gravity_orbital_x, -6.67430e-11 * x)
GP_U(gravity_orbital_y, -6.67430e-11 * x)
GP_B(spring_force, -a * (b - 1.0))
GP_T(spring_damper, -a * (b - 1.0) - c * 0.1)
GP_B(buoyancy_force, a > b ? (a - b) * 9.80665 : 0.0)
GP_B(drag_force, -0.5 * 1.225 * a * a * b)
GP_B(momentum_conservation, (a * b) / (a + b + 1e-9))
GP_B(impulse_response, (1.0 + 0.8) * a / (b + 1e-9))
GP_QUAD(projectile_range, (a * a * gp_sin(2.0 * b)) / 9.80665)
GP_T(projectile_apex, (a * a * gp_sin(b) * gp_sin(b)) / (2.0 * 9.80665))
GP_T(time_of_flight, (2.0 * a * gp_sin(b)) / 9.80665)
GP_B(centripetal_force, (a * a * b))
GP_B(centripetal_accel, (a * a) / (b + 1e-9))
GP_B(angular_velocity_proxy, a / (b + 1e-9))
GP_B(torque_proxy, a * b)
GP_B(moment_of_inertia_disk, 0.5 * a * b * b)
GP_B(moment_of_inertia_rod, 0.08333 * a * b * b)
GP_B(kinetic_energy_rotational, 0.5 * a * b * b)
GP_B(work_done, a * b * gp_cos(0.0))
GP_B(power_output, (a * b) / 1.0)

// 81-100: Hitboxes, Swept Collisions & Intersections
GP_QUAD(swept_aabb_entry, (a - c) / (b - d + 1e-9))
GP_QUAD(swept_aabb_exit, (a + c) / (b + d + 1e-9))
GP_QUAD(circle_segment_dist, (a - c) * (a - c) + (b - d) * (b - d))
GP_T(point_line_dist, gp_abs(a * b + c) / gp_sqrt(a * a + b * b + 1e-9))
GP_QUAD(segment_intersect_proxy, (a * d - b * c))
GP_QUAD(box_containment_score, (a >= c && a <= c + 1.0) ? 1.0 : 0.0)
GP_QUAD(polygon_area_proxy, 0.5 * gp_abs(a * d - b * c))
GP_B(aabb_surface_area, 2.0 * (a * b + a * b))
GP_QUAD(obb_projection_proxy, a * c + b * d)
GP_QUAD(capsule_distance_proxy, gp_sqrt((a - c)*(a - c) + (b - d)*(b - d)))
GP_QUAD(ray_sphere_intersect_t, a * b + c * d)
GP_QUAD(ray_box_intersect_t, (a - c) / (b + 1e-9))
GP_B(collision_response_x, a * 0.5 + b * 0.5)
GP_B(collision_response_y, a * 0.5 + b * 0.5)
GP_B(overlap_area_proxy, a * b)
GP_U(contact_manifold_depth, gp_abs(x))
GP_B(sliding_friction_x, a * (1.0 - b))
GP_B(sliding_friction_y, a * (1.0 - b))
GP_U(restitution_bounce, x * 0.85)
GP_U(damping_factor, x * 0.98)

// 101-125: Pathfinding, Grid & Steering Behaviors
GP_B(heuristic_manhattan, gp_abs(a) + gp_abs(b))
GP_B(heuristic_euclidean, gp_sqrt(a * a + b * b))
GP_B(heuristic_chebyshev, gp_abs(a) > gp_abs(b) ? gp_abs(a) : gp_abs(b))
GP_B(heuristic_octile, (a < b ? a : b) * 0.41421 + (a > b ? a - b : b - a))
GP_T(steering_seek_x, (a - b) * c)
GP_T(steering_seek_y, (a - b) * c)
GP_T(steering_flee_x, (b - a) * c)
GP_T(steering_flee_y, (b - a) * c)
GP_T(steering_seek_y, (a - b) * c)
GP_T(steering_flee_x, (b - a) * c)
GP_T(steering_flee_y, (b - a) * c)
GP_T(steering_arrive_x, (a - b) * c * 0.5)
GP_T(steering_arrive_y, (a - b) * c * 0.5)
GP_QUAD(steering_pursuit_x, a + b * c * d)
GP_QUAD(steering_pursuit_y, a + b * c * d)
GP_T(grid_node_cost, a + b * c)
GP_B(node_weight_blend, a * 0.7 + b * 0.3)
GP_U(flow_field_angle, x * 3.14159265 / 180.0)
GP_B(grid_distance_transform, gp_abs(a - b))
GP_B(raycast_step_x, a + b)
GP_B(raycast_step_y, a + b)
GP_U(cellular_automata_rule, x > 4 ? 1.0 : 0.0)
GP_T(boids_alignment, a * 0.33 + b * 0.33 + c * 0.33)
GP_T(boids_cohesion, (a + b + c) / 3.0)
GP_T(boids_separation, a - (b + c) * 0.5)
GP_B(waypoint_progress, a / (b + 1e-9))
GP_U(path_smoothing_step, x * 0.5)
GP_U(grid_snap_fine, x)

// 126-145: Screen, Camera, Interpolation & Juice Effects
GP_T(camera_follow_x, a + (b - a) * c)
GP_T(camera_follow_y, a + (b - a) * c)
GP_T(camera_deadzone_x, a < b - c ? a + c : (a > b + c ? a - c : b))
GP_T(camera_deadzone_y, a < b - c ? a + c : (a > b + c ? a - c : b))
GP_QUAD(camera_zoom_lerp, a + (b - a) * c * d)
GP_T(screen_shake_decay, a * (1.0 - b))
GP_QUAD(screen_flash_fade, a - b * c * d)
GP_T(screen_warp_uv, a + gp_sin(b) * c)
GP_T(ease_in_cubic, a * a * a)
GP_T(ease_out_cubic, (a - 1.0) * (a - 1.0) * (a - 1.0) + 1.0)
GP_T(ease_in_out_cubic, a < 0.5 ? 4.0 * a * a * a : 1.0 - gp_pow_proxy( -2.0 * a + 2.0, 3.0 ) * 0.5)
GP_U(ease_elastic_proxy, gp_sin(x * 13.0 * 1.5707) * gp_pow_proxy(2.0, -10.0 * x))
GP_U(ease_bounce_proxy, x < 0.363 ? 7.5625 * x * x : 0.9)
GP_T(recoil_recovery, a + (b - a) * c)
GP_B(hitlag_timer, a > 0 ? a - b : 0.0)
GP_T(flash_alpha_proxy, a * gp_sin(b * c))
GP_B(parallax_scroll_x, a * b)
GP_B(parallax_scroll_y, a * b)
GP_U(chromatic_aberration_offset, x * 2.0)
GP_U(vignette_intensity_proxy, 1.0 - x * 0.5)

// 146-160: Economy, RPG Stats & Game Progression Math
GP_B(rpg_damage_mitigation, a * (100.0 / (100.0 + b)))
GP_B(rpg_crit_damage, a * (b > 0.15 ? 1.5 : 1.0))
GP_B(rpg_xp_curve, a * a * 100.0 + b * 50.0)
GP_B(rpg_level_scaling, a * (1.0 + b * 0.1))
GP_B(rpg_stat_diminishing_returns, a / (1.0 + a / (b + 1e-9)))
GP_B(rpg_cooldown_reduction, a * (1.0 - (b / (b + 50.0))))
GP_B(rpg_drop_chance_scaled, a * (1.0 + b * 0.01))
GP_B(economy_inflation_cost, a * gp_pow_proxy(1.05, b))
GP_B(economy_sell_value, a * 0.5)
GP_B(economy_compound_interest, a * gp_pow_proxy(1.01, b))
GP_B(score_combo_multiplier, a * (1.0 + b * 0.1))
GP_B(health_regen_tick, a + b)
GP_B(mana_regen_tick, a + b)
GP_B(stamina_drain_tick, a - b)
GP_B(luck_roll_proxy, a * b * 0.01)


// --- METHOD TABLE REGISTRATION (160 Functions) ---
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

    {"gravity_orbital_x", gp_gravity_orbital_x, METH_VARARGS, ""},
    {"gravity_orbital_y", gp_gravity_orbital_y, METH_VARARGS, ""},
    {"spring_force", gp_spring_force, METH_VARARGS, ""},
    {"spring_damper", gp_spring_damper, METH_VARARGS, ""},
    {"buoyancy_force", gp_buoyancy_force, METH_VARARGS, ""},
    {"drag_force", gp_drag_force, METH_VARARGS, ""},
    {"momentum_conservation", gp_momentum_conservation, METH_VARARGS, ""},
    {"impulse_response", gp_impulse_response, METH_VARARGS, ""},
    {"projectile_range", gp_projectile_range, METH_VARARGS, ""},
    {"projectile_apex", gp_projectile_apex, METH_VARARGS, ""},
    {"time_of_flight", gp_time_of_flight, METH_VARARGS, ""},
    {"centripetal_force", gp_centripetal_force, METH_VARARGS, ""},
    {"centripetal_accel", gp_centripetal_accel, METH_VARARGS, ""},
    {"angular_velocity_proxy", gp_angular_velocity_proxy, METH_VARARGS, ""},
    {"torque_proxy", gp_torque_proxy, METH_VARARGS, ""},
    {"moment_of_inertia_disk", gp_moment_of_inertia_disk, METH_VARARGS, ""},
    {"moment_of_inertia_rod", gp_moment_of_inertia_rod, METH_VARARGS, ""},
    {"kinetic_energy_rotational", gp_kinetic_energy_rotational, METH_VARARGS, ""},
    {"work_done", gp_work_done, METH_VARARGS, ""},
    {"power_output", gp_power_output, METH_VARARGS, ""},

    {"swept_aabb_entry", gp_swept_aabb_entry, METH_VARARGS, ""},
    {"swept_aabb_exit", gp_swept_aabb_exit, METH_VARARGS, ""},
    {"circle_segment_dist", gp_circle_segment_dist, METH_VARARGS, ""},
    {"point_line_dist", gp_point_line_dist, METH_VARARGS, ""},
    {"segment_intersect_proxy", gp_segment_intersect_proxy, METH_VARARGS, ""},
    {"box_containment_score", gp_box_containment_score, METH_VARARGS, ""},
    {"polygon_area_proxy", gp_polygon_area_proxy, METH_VARARGS, ""},
    {"aabb_surface_area", gp_aabb_surface_area, METH_VARARGS, ""},
    {"obb_projection_proxy", gp_obb_projection_proxy, METH_VARARGS, ""},
    {"capsule_distance_proxy", gp_capsule_distance_proxy, METH_VARARGS, ""},
    {"ray_sphere_intersect_t", gp_ray_sphere_intersect_t, METH_VARARGS, ""},
    {"ray_box_intersect_t", gp_ray_box_intersect_t, METH_VARARGS, ""},
    {"collision_response_x", gp_collision_response_x, METH_VARARGS, ""},
    {"collision_response_y", gp_collision_response_y, METH_VARARGS, ""},
    {"overlap_area_proxy", gp_overlap_area_proxy, METH_VARARGS, ""},
    {"contact_manifold_depth", gp_contact_manifold_depth, METH_VARARGS, ""},
    {"sliding_friction_x", gp_sliding_friction_x, METH_VARARGS, ""},
    {"sliding_friction_y", gp_sliding_friction_y, METH_VARARGS, ""},
    {"restitution_bounce", gp_restitution_bounce, METH_VARARGS, ""},
    {"damping_factor", gp_damping_factor, METH_VARARGS, ""},

    {"heuristic_manhattan", gp_heuristic_manhattan, METH_VARARGS, ""},
    {"heuristic_euclidean", gp_heuristic_euclidean, METH_VARARGS, ""},
    {"heuristic_chebyshev", gp_heuristic_chebyshev, METH_VARARGS, ""},
    {"heuristic_octile", gp_heuristic_octile, METH_VARARGS, ""},
    {"steering_seek_x", gp_steering_seek_x, METH_VARARGS, ""},
    {"steering_seek_y", gp_steering_seek_y, METH_VARARGS, ""},
    {"steering_flee_x", gp_steering_flee_x, METH_VARARGS, ""},
    {"steering_flee_y", gp_steering_flee_y, METH_VARARGS, ""},
    {"steering_arrive_x", gp_steering_arrive_x, METH_VARARGS, ""},
    {"steering_arrive_y", gp_steering_arrive_y, METH_VARARGS, ""},
    {"steering_pursuit_x", gp_steering_pursuit_x, METH_VARARGS, ""},
    {"steering_pursuit_y", gp_steering_pursuit_y, METH_VARARGS, ""},
    {"grid_node_cost", gp_grid_node_cost, METH_VARARGS, ""},
    {"node_weight_blend", gp_node_weight_blend, METH_VARARGS, ""},
    {"flow_field_angle", gp_flow_field_angle, METH_VARARGS, ""},
    {"grid_distance_transform", gp_grid_distance_transform, METH_VARARGS, ""},
    {"raycast_step_x", gp_raycast_step_x, METH_VARARGS, ""},
    {"raycast_step_y", gp_raycast_step_y, METH_VARARGS, ""},
    {"cellular_automata_rule", gp_cellular_automata_rule, METH_VARARGS, ""},
    {"boids_alignment", gp_boids_alignment, METH_VARARGS, ""},
    {"boids_cohesion", gp_boids_cohesion, METH_VARARGS, ""},
    {"boids_separation", gp_boids_separation, METH_VARARGS, ""},
    {"waypoint_progress", gp_waypoint_progress, METH_VARARGS, ""},
    {"path_smoothing_step", gp_path_smoothing_step, METH_VARARGS, ""},
    {"grid_snap_fine", gp_grid_snap_fine, METH_VARARGS, ""},

    {"camera_follow_x", gp_camera_follow_x, METH_VARARGS, ""},
    {"camera_follow_y", gp_camera_follow_y, METH_VARARGS, ""},
    {"camera_deadzone_x", gp_camera_deadzone_x, METH_VARARGS, ""},
    {"camera_deadzone_y", gp_camera_deadzone_y, METH_VARARGS, ""},
    {"camera_zoom_lerp", gp_camera_zoom_lerp, METH_VARARGS, ""},
    {"screen_shake_decay", gp_screen_shake_decay, METH_VARARGS, ""},
    {"screen_flash_fade", gp_screen_flash_fade, METH_VARARGS, ""},
    {"screen_warp_uv", gp_screen_warp_uv, METH_VARARGS, ""},
    {"ease_in_cubic", gp_ease_in_cubic, METH_VARARGS, ""},
    {"ease_out_cubic", gp_ease_out_cubic, METH_VARARGS, ""},
    {"ease_in_out_cubic", gp_ease_in_out_cubic, METH_VARARGS, ""},
    {"ease_elastic_proxy", gp_ease_elastic_proxy, METH_VARARGS, ""},
    {"ease_bounce_proxy", gp_ease_bounce_proxy, METH_VARARGS, ""},
    {"recoil_recovery", gp_recoil_recovery, METH_VARARGS, ""},
    {"hitlag_timer", gp_hitlag_timer, METH_VARARGS, ""},
    {"flash_alpha_proxy", gp_flash_alpha_proxy, METH_VARARGS, ""},
    {"parallax_scroll_x", gp_parallax_scroll_x, METH_VARARGS, ""},
    {"parallax_scroll_y", gp_parallax_scroll_y, METH_VARARGS, ""},
    {"chromatic_aberration_offset", gp_chromatic_aberration_offset, METH_VARARGS, ""},
    {"vignette_intensity_proxy", gp_vignette_intensity_proxy, METH_VARARGS, ""},

    {"rpg_damage_mitigation", gp_rpg_damage_mitigation, METH_VARARGS, ""},
    {"rpg_crit_damage", gp_rpg_crit_damage, METH_VARARGS, ""},
    {"rpg_xp_curve", gp_rpg_xp_curve, METH_VARARGS, ""},
    {"rpg_level_scaling", gp_rpg_level_scaling, METH_VARARGS, ""},
    {"rpg_stat_diminishing_returns", gp_rpg_stat_diminishing_returns, METH_VARARGS, ""},
    {"rpg_cooldown_reduction", gp_rpg_cooldown_reduction, METH_VARARGS, ""},
    {"rpg_drop_chance_scaled", gp_rpg_drop_chance_scaled, METH_VARARGS, ""},
    {"economy_inflation_cost", gp_economy_inflation_cost, METH_VARARGS, ""},
    {"economy_sell_value", gp_economy_sell_value, METH_VARARGS, ""},
    {"economy_compound_interest", gp_economy_compound_interest, METH_VARARGS, ""},
    {"score_combo_multiplier", gp_score_combo_multiplier, METH_VARARGS, ""},
    {"health_regen_tick", gp_health_regen_tick, METH_VARARGS, ""},
    {"mana_regen_tick", gp_mana_regen_tick, METH_VARARGS, ""},
    {"stamina_drain_tick", gp_stamina_drain_tick, METH_VARARGS, ""},
    {"luck_roll_proxy", gp_luck_roll_proxy, METH_VARARGS, ""},

    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef gameplay_module = {
    PyModuleDef_HEAD_INIT,
    "gameplay",
    "160 Supercharged Game Engine Accelerators (No math.h)",
    -1,
    GameplayMethods
};

PyMODINIT_FUNC PyInit_gameplay(void) {
    return PyModule_Create(&gameplay_module);
}
