#ifndef LEVELSET_H
#define LEVELSET_H

#include <vector>


#include <QImage>

class LevelSet
{
public:
    LevelSet();
    LevelSet(QImage image);
    float descent_func();
    float grad_plus(int i,int j);
    float grad_minus(int i,int j);
    float central_diff(int i, int j, int direction);
    float second_deriv(int i, int j, int first_direction, int second_direction);
    float first_deriv(int i, int j, int direc, int var);
    float curvature(int i, int j);
    float cost_func(int i, int j);
    void paint_border();
    void mirror_u();
    ~LevelSet();

    QImage m_image_master;
    QImage m_image;
    int m_width;
    int m_height;
    std::vector<std::vector<float> > m_u;

private:
    std::vector<std::vector<float> > m_u_temp;

    std::vector<std::vector<float> > m_central_diff_x;
    std::vector<std::vector<float> > m_central_diff_y;

    std::vector<std::vector<float> > m_deriv_xy;
    std::vector<std::vector<float> > m_deriv_yx;
    std::vector<std::vector<float> > m_deriv_xx;
    std::vector<std::vector<float> > m_deriv_yy;

    std::vector<std::vector<float> > m_du_dx_pos;
    std::vector<std::vector<float> > m_du_dx_neg;
    std::vector<std::vector<float> > m_du_dy_pos;
    std::vector<std::vector<float> > m_du_dy_neg;

    std::vector<std::vector<std::vector<float> > > m_disc_grad;
    std::vector<std::vector<std::vector<std::vector<float> > > > m_disc_hess;

    std::vector<std::vector<float> > m_K;
    std::vector<std::vector<float> > m_F;
};

#endif // LEVELSET_H
