#include "definitions.h"

WeakFormNSNewton::WeakFormNSNewton(bool Stokes, double Reynolds, double time_step, MeshFunctionSharedPtr<double>  x_vel_previous_time,
  MeshFunctionSharedPtr<double>  y_vel_previous_time) : WeakForm<double>(3), Stokes(Stokes),
  Reynolds(Reynolds), time_step(time_step), x_vel_previous_time(x_vel_previous_time),
  y_vel_previous_time(y_vel_previous_time)
{
  /* Jacobian terms - first velocity equation */
  // Time derivative in the first velocity equation
  // and Laplacian divided by Re in the first velocity equation.
  add_matrix_form(new BilinearFormSymVel(0, 0, Stokes, Reynolds, time_step));
  // First part of the convective term in the first velocity equation.
  add_matrix_form(new BilinearFormNonsymVel_0_0(0, 0, Stokes));
  // Second part of the convective term in the first velocity equation.
  add_matrix_form(new BilinearFormNonsymVel_0_1(0, 1, Stokes));
  // Pressure term in the first velocity equation.
  add_matrix_form(new BilinearFormNonsymXVelPressure(0, 2));

  /* Jacobian terms - second velocity equation, continuity equation */
  // Time derivative in the second velocity equation
  // and Laplacian divided by Re in the second velocity equation.
  add_matrix_form(new BilinearFormSymVel(1, 1, Stokes, Reynolds, time_step));
  // First part of the convective term in the second velocity equation.
  add_matrix_form(new BilinearFormNonsymVel_1_0(1, 0, Stokes));
  // Second part of the convective term in the second velocity equation.
  add_matrix_form(new BilinearFormNonsymVel_1_1(1, 1, Stokes));
  // Pressure term in the second velocity equation.
  add_matrix_form(new BilinearFormNonsymYVelPressure(1, 2));

  /* Residual - volumetric */
  // First velocity equation.
  VectorFormNS_0* F_0 = new VectorFormNS_0(0, Stokes, Reynolds, time_step);
  F_0->set_ext(x_vel_previous_time);
  add_vector_form(F_0);
  // Second velocity equation.
  VectorFormNS_1* F_1 = new VectorFormNS_1(1, Stokes, Reynolds, time_step);
  F_1->set_ext(y_vel_previous_time);
  add_vector_form(F_1);
  // Continuity equation.
  VectorFormNS_2* F_2 = new VectorFormNS_2(2);
  add_vector_form(F_2);
}

double WeakFormNSNewton::BilinearFormSymVel::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  double result = int_grad_u_grad_v<double, double>(n, wt, u, v) / Reynolds;
  if (!Stokes)
    result += int_u_v<double, double>(n, wt, u, v) / time_step;
  return result;
}

Ord WeakFormNSNewton::BilinearFormSymVel::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  Ord result = int_grad_u_grad_v<Ord, Ord>(n, wt, u, v) / Reynolds;
  if (!Stokes)
    result += int_u_v<Ord, Ord>(n, wt, u, v) / time_step;
  return result;
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormSymVel::clone() const
{
  return new BilinearFormSymVel(*this);
}

double WeakFormNSNewton::BilinearFormNonsymVel_0_0::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  double result = 0;
  if (!Stokes) {
    Func<double>* xvel_prev_newton = u_ext[0];
    Func<double>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * (xvel_prev_newton->val[i] * u->dx[i] + yvel_prev_newton->val[i] * u->dy[i]
      + u->val[i] * xvel_prev_newton->dx[i]) * v->val[i];
  }
  return result;
}

Ord WeakFormNSNewton::BilinearFormNonsymVel_0_0::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  if (!Stokes) {
    Func<Ord>* xvel_prev_newton = u_ext[0];
    Func<Ord>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * ((xvel_prev_newton->val[i] * u->dx[i] + yvel_prev_newton->val[i]
      * u->dy[i]) * v->val[i] + u->val[i] * v->val[i] * xvel_prev_newton->dx[i]);
  }
  return result;
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymVel_0_0::clone() const
{
  return new BilinearFormNonsymVel_0_0(*this);
}

double WeakFormNSNewton::BilinearFormNonsymVel_0_1::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  double result = 0;
  if (!Stokes) {
    Func<double>* xvel_prev_newton = u_ext[0];
    for (int i = 0; i < n; i++)
      result += wt[i] * u->val[i] * xvel_prev_newton->dy[i] * v->val[i];
  }
  return result;
}

Ord WeakFormNSNewton::BilinearFormNonsymVel_0_1::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  if (!Stokes) {
    Func<Ord>* xvel_prev_newton = u_ext[0];
    for (int i = 0; i < n; i++)
      result += wt[i] * u->val[i] * xvel_prev_newton->dy[i] * v->val[i];
  }
  return result;
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymVel_0_1::clone() const
{
  return new BilinearFormNonsymVel_0_1(*this);
}

double WeakFormNSNewton::BilinearFormNonsymVel_1_0::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  double result = 0;
  if (!Stokes) {
    Func<double>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * u->val[i] * yvel_prev_newton->dx[i] * v->val[i];
  }
  return result;
}

Ord WeakFormNSNewton::BilinearFormNonsymVel_1_0::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  if (!Stokes) {
    Func<Ord>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * u->val[i] * yvel_prev_newton->dx[i] * v->val[i];
  }
  return result;
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymVel_1_0::clone() const
{
  return new BilinearFormNonsymVel_1_0(*this);
}

double WeakFormNSNewton::BilinearFormNonsymVel_1_1::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  double result = 0;
  if (!Stokes) {
    Func<double>* xvel_prev_newton = u_ext[0];
    Func<double>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * (xvel_prev_newton->val[i] * u->dx[i]
      + yvel_prev_newton->val[i] * u->dy[i]
      + u->val[i] * yvel_prev_newton->dy[i]) * v->val[i];
  }
  return result;
}

Ord WeakFormNSNewton::BilinearFormNonsymVel_1_1::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  if (!Stokes) {
    Func<Ord>* xvel_prev_newton = u_ext[0];
    Func<Ord>* yvel_prev_newton = u_ext[1];
    for (int i = 0; i < n; i++)
      result += wt[i] * (xvel_prev_newton->val[i] * u->dx[i]
      + yvel_prev_newton->val[i] * u->dy[i]
      + u->val[i] * yvel_prev_newton->dy[i]) * v->val[i];
  }
  return result;
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymVel_1_1::clone() const
{
  return new BilinearFormNonsymVel_1_1(*this);
}

double WeakFormNSNewton::BilinearFormNonsymXVelPressure::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  return -int_u_dvdx<double, double>(n, wt, u, v);
}

Ord WeakFormNSNewton::BilinearFormNonsymXVelPressure::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  return -int_u_dvdx<Ord, Ord>(n, wt, u, v);
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymXVelPressure::clone() const
{
  return new BilinearFormNonsymXVelPressure(*this);
}

double WeakFormNSNewton::BilinearFormNonsymYVelPressure::value(int n, double *wt, Func<double> *u_ext[], Func<double> *u, Func<double> *v,
  Geom<double> *e, Func<double>* *ext) const
{
  return -int_u_dvdy<double, double>(n, wt, u, v);
}

Ord WeakFormNSNewton::BilinearFormNonsymYVelPressure::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *u, Func<Ord> *v, Geom<Ord> *e,
  Func<Ord>* *ext) const
{
  return -int_u_dvdy<Ord, Ord>(n, wt, u, v);
}

MatrixFormVol<double>* WeakFormNSNewton::BilinearFormNonsymYVelPressure::clone() const
{
  return new BilinearFormNonsymYVelPressure(*this);
}

double WeakFormNSNewton::VectorFormNS_0::value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e,
  Func<double>* *ext) const
{
  double result = 0;
  Func<double>* xvel_prev_time = ext[0];
  Func<double>* xvel_prev_newton = u_ext[0];
  Func<double>* yvel_prev_newton = u_ext[1];
  Func<double>* p_prev_newton = u_ext[2];
  for (int i = 0; i < n; i++)
    result += wt[i] * ((xvel_prev_newton->dx[i] * v->dx[i] + xvel_prev_newton->dy[i] * v->dy[i]) / Reynolds
    - (p_prev_newton->val[i] * v->dx[i]));
  if (!Stokes)
    for (int i = 0; i < n; i++)
      result += wt[i] * (((xvel_prev_newton->val[i] - xvel_prev_time->val[i]) * v->val[i] / time_step)
      + ((xvel_prev_newton->val[i] * xvel_prev_newton->dx[i]
      + yvel_prev_newton->val[i] * xvel_prev_newton->dy[i]) * v->val[i]));
  return result;
}

Ord WeakFormNSNewton::VectorFormNS_0::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  Func<Ord>* xvel_prev_time = ext[0];
  Func<Ord>* xvel_prev_newton = u_ext[0];
  Func<Ord>* yvel_prev_newton = u_ext[1];
  Func<Ord>* p_prev_newton = u_ext[2];
  for (int i = 0; i < n; i++)
    result += wt[i] * ((xvel_prev_newton->dx[i] * v->dx[i] + xvel_prev_newton->dy[i] * v->dy[i]) / Reynolds
    - (p_prev_newton->val[i] * v->dx[i]));
  if (!Stokes)
    for (int i = 0; i < n; i++)
      result += wt[i] * (((xvel_prev_newton->val[i] - xvel_prev_time->val[i]) * v->val[i] / time_step)
      + ((xvel_prev_newton->val[i] * xvel_prev_newton->dx[i]
      + yvel_prev_newton->val[i] * xvel_prev_newton->dy[i]) * v->val[i]));
  return result;
}

VectorFormVol<double>* WeakFormNSNewton::VectorFormNS_0::clone() const
{
  return new VectorFormNS_0(*this);
}

VectorFormVol<double>* WeakFormNSNewton::VectorFormNS_1::clone() const
{
  return new VectorFormNS_1(*this);
}

double WeakFormNSNewton::VectorFormNS_1::value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e,
  Func<double>* *ext) const
{
  double result = 0;
  Func<double>* yvel_prev_time = ext[0];
  Func<double>* xvel_prev_newton = u_ext[0];
  Func<double>* yvel_prev_newton = u_ext[1];
  Func<double>* p_prev_newton = u_ext[2];
  for (int i = 0; i < n; i++)
    result += wt[i] * ((yvel_prev_newton->dx[i] * v->dx[i] + yvel_prev_newton->dy[i] * v->dy[i]) / Reynolds
    - (p_prev_newton->val[i] * v->dy[i]));
  if (!Stokes)
    for (int i = 0; i < n; i++)
      result += wt[i] * (((yvel_prev_newton->val[i] - yvel_prev_time->val[i]) * v->val[i] / time_step)
      + ((xvel_prev_newton->val[i] * yvel_prev_newton->dx[i]
      + yvel_prev_newton->val[i] * yvel_prev_newton->dy[i]) * v->val[i]));
  return result;
}

Ord WeakFormNSNewton::VectorFormNS_1::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  Func<Ord>* yvel_prev_time = ext[0];
  Func<Ord>* xvel_prev_newton = u_ext[0];
  Func<Ord>* yvel_prev_newton = u_ext[1];
  Func<Ord>* p_prev_newton = u_ext[2];
  for (int i = 0; i < n; i++)
    result += wt[i] * ((xvel_prev_newton->dx[i] * v->dx[i] + xvel_prev_newton->dy[i] * v->dy[i]) / Reynolds
    - (p_prev_newton->val[i] * v->dx[i]));
  if (!Stokes)
    for (int i = 0; i < n; i++)
      result += wt[i] * (((yvel_prev_newton->val[i] - yvel_prev_time->val[i]) * v->val[i] / time_step)
      + ((xvel_prev_newton->val[i] * xvel_prev_newton->dx[i]
      + yvel_prev_newton->val[i] * xvel_prev_newton->dy[i]) * v->val[i]));
  return result;
}

double WeakFormNSNewton::VectorFormNS_2::value(int n, double *wt, Func<double> *u_ext[], Func<double> *v, Geom<double> *e,
  Func<double>* *ext) const
{
  double result = 0;
  Func<double>* xvel_prev_newton = u_ext[0];
  Func<double>* yvel_prev_newton = u_ext[1];

  for (int i = 0; i < n; i++)
    result += wt[i] * (xvel_prev_newton->dx[i] * v->val[i] + yvel_prev_newton->dy[i] * v->val[i]);
  return result;
}

VectorFormVol<double>* WeakFormNSNewton::VectorFormNS_2::clone() const
{
  return new VectorFormNS_2(*this);
}

Ord WeakFormNSNewton::VectorFormNS_2::ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e, Func<Ord>* *ext) const
{
  Ord result = Ord(0);
  Func<Ord>* xvel_prev_newton = u_ext[0];
  Func<Ord>* yvel_prev_newton = u_ext[1];

  for (int i = 0; i < n; i++)
    result += wt[i] * (xvel_prev_newton->dx[i] * v->val[i] + yvel_prev_newton->dy[i] * v->val[i]);
  return result;
}

EssentialBoundaryCondition<double>::EssentialBCValueType EssentialBCNonConstX::get_value_type() const
{
  return BC_FUNCTION;
}

double EssentialBCNonConstX::value(double x, double y) const
{
  double velocity;
  if (current_time <= startup_time) velocity = vel * current_time / startup_time;
  else velocity = vel;
  double alpha = std::atan2(x, y);
  double xvel = velocity*std::cos(alpha);
  return xvel;
}

EssentialBoundaryCondition<double>::EssentialBCValueType EssentialBCNonConstY::get_value_type() const
{
  return BC_FUNCTION;
}

double EssentialBCNonConstY::value(double x, double y) const
{
  double velocity;
  if (current_time <= startup_time) velocity = vel * current_time / startup_time;
  else velocity = vel;
  double alpha = std::atan2(x, y);
  double yvel = -velocity*std::sin(alpha);
  return yvel;
}