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
    void paint_border();
    void mirror_u();
    void calculate_parameters();
    void unitize_u();
    ~LevelSet();

    QImage m_image_master;
    QImage m_image;
    int m_width;
    int m_height;
    std::vector<std::vector<float> > m_u;
    float m_coef_length;
    float m_coef_mean_inside;
    float m_coef_mean_outside;
    float m_coef_variance_inside;
    float m_coef_variance_outside;
    float m_coef_area;
    float m_coef_com;
    float m_coef_pixel;
    int m_iterations;

private:
    float grad_plus(int i,int j);
    float grad_minus(int i,int j);
    float central_diff(int i, int j, int direction);
    float second_deriv(int i, int j, int first_direction, int second_direction);
    float first_deriv(int i, int j, int direc, int var);

    float curvature(int i, int j);
    float cost_func(int i, int j);
    float area_func();
    float mean_inside_func(int i, int j);
    float mean_outside_func(int i, int j);
    float variance_inside_func(int i, int j);
    float variance_outside_func(int i, int j);
    float com_func(int i, int j);
    float segmentation_func(int i, int j);

    void calculate_area();
    void calculate_mean();
    void calculate_variance();
    void calculate_com();

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

    // COM needs a 2-tuple (x,y) coordinate
    // Mean & Variance need a 3-tuple (RGB)
    // Original image measures
    float m_orig_area;
    std::vector<float> m_orig_com;
    std::vector<float> m_orig_mean_inside;
    std::vector<float> m_orig_mean_outside;
    std::vector<float> m_orig_variance_inside;
    std::vector<float> m_orig_variance_outside;

    // Evolving measures
    float m_area_inside;
    float m_area_outside;
    std::vector<float> m_com;
    std::vector<float> m_mean_inside;
    std::vector<float> m_mean_outside;
    std::vector<float> m_variance_inside;
    std::vector<float> m_variance_outside;
};

#endif // LEVELSET_H
