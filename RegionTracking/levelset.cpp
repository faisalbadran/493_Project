#include "levelset.h"

#include <math.h>

#include <QImage>

LevelSet::LevelSet()
{

}

LevelSet::LevelSet(QImage image)
{
    m_image_master = image;
    m_image = image;

    // Initialize derivative arrays

    m_width = (int)image.width() + 2;
    m_height = (int)image.height() + 2;

    m_u.resize(m_width, std::vector<float>(m_height, 0.0));
    m_u_temp.resize(m_width, std::vector<float>(m_height, 0.0));

    m_central_diff_x.resize(m_width, std::vector<float>(m_height, 0.0));
    m_central_diff_y.resize(m_width, std::vector<float>(m_height, 0.0));

    m_deriv_xy.resize(m_width, std::vector<float>(m_height, 0.0));
    m_deriv_yx.resize(m_width, std::vector<float>(m_height, 0.0));
    m_deriv_xx.resize(m_width, std::vector<float>(m_height, 0.0));
    m_deriv_yy.resize(m_width, std::vector<float>(m_height, 0.0));

    m_du_dx_pos.resize(m_width, std::vector<float>(m_height, 0.0));
    m_du_dx_neg.resize(m_width, std::vector<float>(m_height, 0.0));
    m_du_dy_pos.resize(m_width, std::vector<float>(m_height, 0.0));
    m_du_dy_neg.resize(m_width, std::vector<float>(m_height, 0.0));

    m_disc_grad.resize(6, std::vector<std::vector<float> >(m_width, std::vector<float>(m_height, 0.0)));
    m_disc_hess.resize(2, std::vector<std::vector<std::vector<float> > >(2, std::vector<std::vector<float> >(m_width, std::vector<float>(m_height, 0.0))));

    m_K.resize(m_width, std::vector<float>(m_height, 0.0));
    m_F.resize(m_width, std::vector<float>(m_height, 0.0));
}

LevelSet::~LevelSet()
{

}

float LevelSet::descent_func()
{
    // Calculate derivatives
    for(int i=1; i < m_width-1; i++)
    {
        for(int j=1; j < m_height-1; j++)
        {
            m_u_temp.at(i).at(j) = m_u.at(i).at(j);
            m_central_diff_x.at(i).at(j) = central_diff(i,j,1);
            m_central_diff_y.at(i).at(j) = central_diff(i,j,0);
            m_deriv_xy.at(i).at(j) = second_deriv(i,j,1,0);
            m_deriv_yx.at(i).at(j) = second_deriv(i,j,0,1);
            m_deriv_yy.at(i).at(j) = second_deriv(i,j,0,0);
            m_deriv_xx.at(i).at(j)= second_deriv(i,j,1,1);
            m_du_dx_pos.at(i).at(j) = first_deriv(i,j,1,1);
            m_du_dx_neg.at(i).at(j) = first_deriv(i,j,-1,1);
            m_du_dy_pos.at(i).at(j) = first_deriv(i,j,1,0);
            m_du_dy_neg.at(i).at(j) = first_deriv(i,j,-1,0);
        }
    }

    mirror_u();

    // Discrete Gradient
    m_disc_grad.at(0) = m_du_dx_pos;
    m_disc_grad.at(1) = m_du_dx_neg;
    m_disc_grad.at(2) = m_du_dy_pos;
    m_disc_grad.at(3) = m_du_dy_neg;
    m_disc_grad.at(4) = m_central_diff_x;
    m_disc_grad.at(5) = m_central_diff_y;

    // Discrete Hessian
    m_disc_hess.at(0).at(0) = m_deriv_xx;
    m_disc_hess.at(0).at(1) = m_deriv_xy;
    m_disc_hess.at(1).at(0) = m_deriv_yx;
    m_disc_hess.at(1).at(1) = m_deriv_yy;

    float max_F = 0.0;

    float epsilon = 1000.0;

    for(int i=1; i < m_width-1; i++)
    {
        for(int j=1; j < m_height-1; j++)
        {
            m_K.at(i).at(j) = curvature(i, j);
            m_F.at(i).at(j) = cost_func(i, j)/500.0;

            if(fabs(epsilon*m_K.at(i).at(j) + m_F.at(i).at(j)) > max_F)
            {
                max_F = fabs(epsilon*m_K.at(i).at(j) + m_F.at(i).at(j));
            }
        }
    }

    if(max_F == 0)
    {
        max_F = 10000;
    }

    float delta_t = 1/fmin(max_F, 10000.0);

    for(int i=1; i < m_width-1; i++)
    {
        for(int j=1; j < m_height-1; j++)
        {
            m_u.at(i).at(j) = m_u_temp.at(i).at(j) + delta_t * (epsilon*m_K.at(i).at(j) * pow(pow(m_disc_grad.at(4).at(i).at(j),2) + pow(m_disc_grad.at(5).at(i).at(j),2),0.5) - (fmax(m_F.at(i).at(j),0)*grad_plus(i,j)+fmin(m_F.at(i).at(j),0)*grad_minus(i,j)));

            // Limit size of u
            if(fabs(m_u.at(i).at(j)) > 10000)
            {
                m_u.at(i).at(j) = m_u.at(i).at(j)/fabs(m_u.at(i).at(j))*10000;
            }
        }
    }

    mirror_u();

    return delta_t;
}

float LevelSet::grad_plus(int i, int j){

    float g = pow(fmax(0.0, m_disc_grad.at(1).at(i).at(j)),2);
    g += pow(fmax(0.0, m_disc_grad.at(3).at(i).at(j)),2);
    g += pow(fmin(0.0, m_disc_grad.at(0).at(i).at(j)),2);
    g += pow(fmin(0.0, m_disc_grad.at(2).at(i).at(j)),2);

    return pow(g,0.5);
}

float LevelSet::grad_minus(int i,int j){

    float g = pow(fmin(0.0, m_disc_grad.at(1).at(i).at(j)),2);
    g += pow(fmin(0.0, m_disc_grad.at(3).at(i).at(j)),2);
    g += pow(fmax(0.0, m_disc_grad.at(0).at(i).at(j)),2);
    g += pow(fmax(0.0, m_disc_grad.at(2).at(i).at(j)),2);

    return pow(g,0.5);
}

float LevelSet::central_diff(int i, int j, int direction){

    /*
     * direction == 0 is dy
     * direction == 1 is dx
     */

    if(direction == 0)
    {
        return (m_u.at(i).at(j+1) - m_u.at(i).at(j-1)) / 2;
    }
    if(direction == 1)
    {
        return (m_u.at(i+1).at(j) - m_u.at(i-1).at(j)) / 2;
    }

    return 0;
}

float LevelSet::second_deriv(int i, int j, int first_direction, int second_direction)
{
    /*
     * direction == 0 is dy
     * direction == 1 is dx
     */

    if(first_direction == 0)
    {
        if(second_direction == 1)
        {
            return (m_u.at(i+1).at(j+1) - m_u.at(i-1).at(j+1) - m_u.at(i+1).at(j-1) + m_u.at(i-1).at(j-1))/4;
        }
        else if(second_direction == 0)
        {
            return m_u.at(i).at(j+1) - 2*m_u.at(i).at(j) + m_u.at(i).at(j-1);
        }
    }

    else if(first_direction == 1)
    {
        if(second_direction == 0)
        {
            return (m_u.at(i+1).at(j+1) - m_u.at(i-1).at(j+1) - m_u.at(i+1).at(j-1) + m_u.at(i-1).at(j-1))/4;
        }
        else if(second_direction == 1)
        {
            return m_u.at(i+1).at(j) - 2*m_u.at(i).at(j) + m_u.at(i-1).at(j);
        }
    }

    return 0;
}

float LevelSet::first_deriv(int i, int j, int direc, int var)
{
    /* var
     * == 0 , y
     * == 1 , x
     * direc
     * > 0 , forward difference
     * < 0 , backward difference
     */
    if(var == 0)
    {
        if(direc < 0)
        {
            return m_u.at(i).at(j) - m_u.at(i).at(j-1);
        }
        if(direc > 0)
        {
            return m_u.at(i).at(j + 1) - m_u.at(i).at(j);
        }
    }
    if(var == 1)
    {
        if(direc < 0)
        {
            return m_u.at(i).at(j) - m_u.at(i-1).at(j);
        }
        if(direc > 0)
        {
            return m_u.at(i+1).at(j) - m_u.at(i).at(j);
        }
    }

    return 0;
}

// Curvature calculation
float LevelSet::curvature(int i, int j)
{
    float denom = pow(pow(m_disc_grad.at(4).at(i).at(j),2) + pow(m_disc_grad.at(5).at(i).at(j),2), 1.5);
    float num = m_disc_hess.at(0).at(0).at(i).at(j)* pow(m_disc_grad.at(5).at(i).at(j),2) - 2 * m_disc_grad.at(5).at(i).at(j) * m_disc_grad.at(4).at(i).at(j) * m_disc_hess.at(0).at(1).at(i).at(j) + m_disc_hess.at(1).at(1).at(i).at(j) * pow(m_disc_grad.at(4).at(i).at(j),2);

    if(denom == 0)
    {
        return 0.0;
    }

    return num / denom;
}

// For image segmentation functional
float LevelSet::cost_func(int i, int j)
{
    QRgb pixel = m_image.pixel(i-1, j-1);

    return pow(qRed(pixel) - 255,2) - pow(qRed(pixel) - 0,2) + pow(qGreen(pixel) - 255,2) - pow(qGreen(pixel) - 0,2) + pow(qBlue(pixel) - 255,2) - pow(qBlue(pixel) - 0,2);
}

void LevelSet::paint_border()
{
    // Paint blue border if there are any white neighbouring pixels

    int white_neighbours = 0;

    m_image = m_image_master;

    for(int i = 1; i < m_width-1; i++)
    {
        for(int j = 1; j < m_height-1; j++)
        {
            white_neighbours = 0;

            if(m_u.at(i).at(j) >= 0)
            {
                if (m_u.at(i-1).at(j) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i+1).at(j) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i).at(j-1) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i).at(j+1) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i-1).at(j-1) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i-1).at(j+1) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i+1).at(j-1) < 0)
                {
                    white_neighbours += 1;
                }
                if (m_u.at(i+1).at(j+1) < 0)
                {
                    white_neighbours += 1;
                }
            }

            if(white_neighbours > 0)
            {
                m_image.setPixel(i-1, j-1, qRgb(0,0,255));
            }
        }
    }
}

void LevelSet::mirror_u(){
    // Mirror image the border
    for(int i=1; i < m_width; i++)
    {
        m_u.at(i).at(0) = m_u.at(i).at(1);
        m_u.at(i).at(m_height-1) = m_u.at(i).at(m_height-2);
    }

    for(int j=1; j < m_height; j++)
    {
        m_u.at(0).at(j) = m_u.at(1).at(j);
        m_u.at(m_width-1).at(j) = m_u.at(m_width-2).at(j);
    }

    // Four corners
    m_u.at(0).at(0) = m_u.at(1).at(1);
    m_u.at(0).at(m_height-1) = m_u.at(1).at(m_height-2);
    m_u.at(m_width-1).at(0) = m_u.at(m_width-2).at(1);
    m_u.at(m_width-1).at(m_height-1) = m_u.at(m_width-2).at(m_height-2);
}
