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

    // Initialize parameters to 0
    m_orig_com.resize(2,0.0);
    m_orig_mean_inside.resize(3,0.0);
    m_orig_mean_outside.resize(3,0.0);
    m_orig_variance_inside.resize(3,0.0);
    m_orig_variance_outside.resize(3,0.0);

    m_com.resize(2,0.0);
    m_mean_inside.resize(3,0.0);
    m_mean_outside.resize(3,0.0);
    m_variance_inside.resize(3,0.0);
    m_variance_outside.resize(3,0.0);
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

    // Calculate parameters before the cost_func is called
    calculate_area();
    calculate_com();
    calculate_mean();
    calculate_variance();

    for(int i=1; i < m_width-1; i++)
    {
        for(int j=1; j < m_height-1; j++)
        {
            m_K.at(i).at(j) = curvature(i, j);
            m_F.at(i).at(j) = cost_func(i, j);

            if(fabs(m_coef_length*m_K.at(i).at(j) + m_F.at(i).at(j)) > max_F)
            {
                max_F = fabs(m_coef_length*m_K.at(i).at(j) + m_F.at(i).at(j));
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
            m_u.at(i).at(j) = m_u_temp.at(i).at(j) + delta_t * (m_coef_length*m_K.at(i).at(j) * pow(pow(m_disc_grad.at(4).at(i).at(j),2) + pow(m_disc_grad.at(5).at(i).at(j),2),0.5) - (fmax(m_F.at(i).at(j),0)*grad_plus(i,j)+fmin(m_F.at(i).at(j),0)*grad_minus(i,j)));

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

// Overall functional calls sub functionals
float LevelSet::cost_func(int i, int j)
{
    float area_func_val = 0;
    float mean_inside_func_val = 0;
    float mean_outside_func_val = 0;
    float variance_inside_func_val = 0;
    float variance_outside_func_val = 0;
    float com_func_val = 0;

    if(m_coef_area != 0){
        area_func_val = -m_coef_area*area_func();
    }
    if(m_coef_mean_inside != 0 ){
        mean_inside_func_val = -m_coef_mean_inside*mean_inside_func(i,j);
    }
    if(m_coef_mean_outside != 0 ){
        mean_outside_func_val = -m_coef_mean_outside*mean_outside_func(i,j);
    }
    if(m_coef_variance_inside != 0){
        variance_inside_func_val = -m_coef_variance_inside*variance_inside_func(i,j);
    }
    if(m_coef_variance_outside != 0){
        variance_outside_func_val = -m_coef_variance_outside*variance_outside_func(i,j);
    }
    if(m_coef_com != 0){
        com_func_val = -m_coef_com*com_func(i,j);
    }

    return area_func_val + mean_inside_func_val + mean_outside_func_val + variance_inside_func_val + variance_outside_func_val + com_func_val;
}

// Area functional
float LevelSet::area_func(){
    return 2*(m_area_inside-m_orig_area);
}

// Mean functional
float LevelSet::mean_inside_func(int i, int j){
    QRgb pixel = m_image.pixel(i-1, j-1);

    float val = 0;

    val += 2*(m_mean_inside.at(0) - m_orig_mean_inside.at(0))*(qRed(pixel)-m_mean_inside.at(0))/m_area_inside;
    val += 2*(m_mean_inside.at(1) - m_orig_mean_inside.at(1))*(qGreen(pixel)-m_mean_inside.at(1))/m_area_inside;
    val += 2*(m_mean_inside.at(2) - m_orig_mean_inside.at(2))*(qBlue(pixel)-m_mean_inside.at(2))/m_area_inside;

    return val;
}

float LevelSet::mean_outside_func(int i, int j){
    QRgb pixel = m_image.pixel(i-1, j-1);

    float val = 0;

    val += 2*(m_mean_outside.at(0) - m_orig_mean_outside.at(0))*(qRed(pixel)-m_mean_outside.at(0))/m_area_outside;
    val += 2*(m_mean_outside.at(1) - m_orig_mean_outside.at(1))*(qGreen(pixel)-m_mean_outside.at(1))/m_area_outside;
    val += 2*(m_mean_outside.at(2) - m_orig_mean_outside.at(2))*(qBlue(pixel)-m_mean_outside.at(2))/m_area_outside;

    return val;
}

// Variance functional

float LevelSet::variance_inside_func(int i, int j){
    QRgb pixel = m_image.pixel(i-1, j-1);

    float val = 0;

    val += 2*(m_variance_inside.at(0) - m_orig_variance_inside.at(0))*(qRed(pixel)*qRed(pixel)-m_variance_inside.at(0))/m_area_inside;
    val += 2*(m_variance_inside.at(1) - m_orig_variance_inside.at(1))*(qGreen(pixel)*qGreen(pixel)-m_variance_inside.at(1))/m_area_inside;
    val += 2*(m_variance_inside.at(2) - m_orig_variance_inside.at(2))*(qBlue(pixel)*qBlue(pixel)-m_variance_inside.at(2))/m_area_inside;

    return val;
}

float LevelSet::variance_outside_func(int i, int j){
    QRgb pixel = m_image.pixel(i-1, j-1);

    float val = 0;

    val += 2*(m_variance_outside.at(0) - m_orig_variance_outside.at(0))*(qRed(pixel)*qRed(pixel)-m_variance_outside.at(0))/m_area_outside;
    val += 2*(m_variance_outside.at(1) - m_orig_variance_outside.at(1))*(qGreen(pixel)*qGreen(pixel)-m_variance_outside.at(1))/m_area_outside;
    val += 2*(m_variance_outside.at(2) - m_orig_variance_outside.at(2))*(qBlue(pixel)*qBlue(pixel)-m_variance_outside.at(2))/m_area_outside;

    return val;
}

// COM functional

float LevelSet::com_func(int i, int j){
    float val = 0;

    val += 2*(m_com.at(0) - m_orig_com.at(0))*(i - m_com.at(0))/m_area_inside;
    val += 2*(m_com.at(1) - m_orig_com.at(1))*(j - m_com.at(1))/m_area_inside;

    return val;
}

// Functional to segment image on color
float LevelSet::segmentation_func(int i, int j)
{
    QRgb pixel = m_image.pixel(i-1, j-1);

    return pow(qRed(pixel) - m_mean_inside.at(0),2) + pow(qGreen(pixel) - m_mean_inside.at(1),2) + pow(qBlue(pixel) - m_mean_inside.at(2),2);
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

void LevelSet::calculate_parameters(){
    calculate_area();
    calculate_com();
    calculate_mean();
    calculate_variance();

    m_orig_area = m_area_inside;
    m_orig_com = m_com;
    m_orig_mean_inside = m_mean_inside;
    m_orig_mean_outside = m_mean_outside;
    m_orig_variance_inside = m_variance_inside;
    m_orig_variance_outside = m_variance_outside;
}

void LevelSet::calculate_area(){
    float area_inside = 0;
    float area_outside = 0;

    // Calculate area for
    // inside and outside the region (inside is positive u)
    for(int i = 0; i < m_width; i++){
        for(int j = 0; j < m_height; j++){
            if(m_u.at(i).at(j) >= 0){
                area_inside += 1;
            }
            else{
                area_outside += 1;
            }
        }
    }

    m_area_inside = area_inside;
    m_area_outside = area_outside;
}

void LevelSet::calculate_com(){
    float area = 0;
    std::vector<float> com;
    com.resize(2,0.0);

    // Calculate com for the x,y coordiantes value for
    // inside the region (inside is positive u)
    for(int i = 0; i < m_width; i++){
        for(int j = 0; j < m_height; j++){
            if(m_u.at(i).at(j) >= 0){
                area += 1;
                com.at(0) += i;
                com.at(1) += j;
            }
        }
    }

    m_com.at(0) = com.at(0)/area;
    m_com.at(1) = com.at(1)/area;
}

void LevelSet::calculate_mean(){
    float area_inside = 0;
    float area_outside = 0;
    std::vector<float> mean_inside;
    std::vector<float> mean_outside;
    mean_inside.resize(3,0.0);
    mean_outside.resize(3,0.0);

    // Calculate mean for the RGB value for both
    // inside and outside the region (inside is positive u)
    for(int i = 1; i < m_width - 1; i++){
        for(int j = 1; j < m_height - 1; j++){
            QRgb pixel = m_image.pixel(i-1, j-1);

            if(m_u.at(i).at(j) >= 0){
                area_inside += 1;
                mean_inside.at(0) += qRed(pixel);
                mean_inside.at(1) += qGreen(pixel);
                mean_inside.at(2) += qBlue(pixel);
            }
            else{
                area_outside += 1;
                mean_outside.at(0) += qRed(pixel);
                mean_outside.at(1) += qGreen(pixel);
                mean_outside.at(2) += qBlue(pixel);
            }
        }
    }

    m_mean_inside.at(0) = mean_inside.at(0)/area_inside;
    m_mean_inside.at(1) = mean_inside.at(1)/area_inside;
    m_mean_inside.at(2) = mean_inside.at(2)/area_inside;

    m_mean_outside.at(0) = mean_outside.at(0)/area_outside;
    m_mean_outside.at(1) = mean_outside.at(1)/area_outside;
    m_mean_outside.at(2) = mean_outside.at(2)/area_outside;
}

void LevelSet::calculate_variance(){
    float area_inside = 0;
    float area_outside = 0;
    std::vector<float> variance_inside;
    std::vector<float> variance_outside;
    variance_inside.resize(3,0.0);
    variance_outside.resize(3,0.0);

    // Calculate mean for the RGB value for both
    // inside and outside the region (inside is positive u)
    for(int i = 1; i < m_width - 1; i++){
        for(int j = 1; j < m_height - 1; j++){
            QRgb pixel = m_image.pixel(i-1, j-1);

            if(m_u.at(i).at(j) >= 0){
                area_inside += 1;
                variance_inside.at(0) += qRed(pixel)*qRed(pixel);
                variance_inside.at(1) += qGreen(pixel)*qGreen(pixel);
                variance_inside.at(2) += qBlue(pixel)*qBlue(pixel);
            }
            else{
                area_outside += 1;
                variance_outside.at(0) += qRed(pixel)*qRed(pixel);
                variance_outside.at(1) += qGreen(pixel)*qGreen(pixel);
                variance_outside.at(2) += qBlue(pixel)*qBlue(pixel);
            }
        }
    }

    m_variance_inside.at(0) = variance_inside.at(0)/area_inside;
    m_variance_inside.at(1) = variance_inside.at(1)/area_inside;
    m_variance_inside.at(2) = variance_inside.at(2)/area_inside;

    m_variance_outside.at(0) = variance_outside.at(0)/area_outside;
    m_variance_outside.at(1) = variance_outside.at(1)/area_outside;
    m_variance_outside.at(2) = variance_outside.at(2)/area_outside;
}

void LevelSet::unitize_u(){
    for(int i = 0; i < m_width; i++){
        for(int j = 0; j < m_height; j++){
            m_u.at(i).at(j) = m_u.at(i).at(j)/fabs(m_u.at(i).at(j));
        }
    }
}
