def descent_func(pic, u):

    # Initialize derivative arrays
    u_temp = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    central_diff_x = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    central_diff_y = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    deriv_xy = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    deriv_xx = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    deriv_yy = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    du_dx_pos = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    du_dx_neg = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    du_dy_pos = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    du_dy_neg = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    K = [[0 for j in range(len(u[i]))] for i in range(len(u))]

    # Calculate derivatives
    for i in range(len(u)):
        for j in range(len(u[0])):
            u_temp[i][j] = u[i][j]
            central_diff_x[i][j] = central_diff(u,i,j,0)
            central_diff_y[i][j] = central_diff(u,i,j,1)
            deriv_xy[i][j] = second_deriv(u,i,j,0,1)
            deriv_yy[i][j] = second_deriv(u,i,j,1,1)
            deriv_xx[i][j] = second_deriv(u,i,j,0,0)
            du_dx_pos[i][j] = first_deriv(u,i,j,1,0)
            du_dx_neg[i][j] = first_deriv(u,i,j,-1,0)
            du_dy_pos[i][j] = first_deriv(u,i,j,1,1)
            du_dy_neg[i][j] = first_deriv(u,i,j,-1,1)

    #Discrete Gradient
    disc_grad = [[],[],[],[],[],[]]
    disc_grad[0] = du_dx_pos
    disc_grad[1] = du_dx_neg
    disc_grad[2] = du_dy_pos
    disc_grad[3] = du_dy_neg
    disc_grad[4] = central_diff_x
    disc_grad[5] = central_diff_y

    #Discrete Hessian
    disc_hess = [[[],[]],[[],[]]]
    disc_hess[0][0] = deriv_xx
    disc_hess[0][1] = deriv_xy
    disc_hess[1][0] = deriv_xy
    disc_hess[1][1] = deriv_yy

    F = [[0 for j in range(len(u[i]))] for i in range(len(u))]
    max_F = 0

    epsilon = 10000

    for i in range(1, len(u) - 1):
        for j in range(1, len(u[i]) - 1):
            K[i][j] = curvature(u, i, j, disc_grad, disc_hess)
            F[i][j] = cost_func(i, j, pic)/10000

            if abs(epsilon*K[i][j] + F[i][j]) > max_F:
                max_F = abs(epsilon*K[i][j] + F[i][j])

    # Mirror image the border
    for i in range(len(u)):
        u[i][0] = u[i][1]
        u[i][-1] = u[i][-2]

    for j in range(len(u[0])):
        u[0][j] = u[1][j]
        u[-1][j] = u[-2][j]

    # Four corners
    u[0][0] = u[1][1]
    u[0][-1] = u[1][-2]
    u[-1][0] = u[-2][1]
    u[-1][-1] = u[-2][-2]

    if max_F == 0:
        max_F = 1000

    delta_t = 0.5/min(max_F, 1000000)

    for i in range(len(u)):
        for j in range(len(u[i])):
            u[i][j] = u_temp[i][j] + delta_t * (epsilon*K[i][j] * ((disc_grad[4][i][j])**2 + (disc_grad[5][i][j])**2)**0.5 + (max(F[i][j],0)*grad_plus(i,j,disc_grad)+min(F[i][j],0)*grad_minus(i,j,disc_grad)))
            if abs(u[i][j]) > 10000:
                u[i][j] = u[i][j]/abs(u[i][j])*10000

    return delta_t

def grad_plus(i,j,disc_grad):

    g = max([0, disc_grad[1][i][j]])**2
    g += max([0, disc_grad[3][i][j]])**2
    g += min([0, disc_grad[0][i][j]])**2
    g += min([0, disc_grad[2][i][j]])**2

    return g**0.5

def grad_minus(i,j,disc_grad):

    g = min([0, disc_grad[1][i][j]])**2
    g += min([0, disc_grad[3][i][j]])**2
    g += max([0, disc_grad[0][i][j]])**2
    g += max([0, disc_grad[2][i][j]])**2

    return g**0.5

def central_diff(u,i,j,direction):

    # direction == 0 is dy
    # direction == 1 is dx
    if direction == 0:
        if j >= len(u[i]) - 1:
            return (-1 - u[i][j-1]) / 2
        else:
            if j <= 0:
                return (u[i][j+1] + 1) / 2
            else:
                return (u[i][j+1] - u[i][j-1]) / 2
    if direction == 1:
        if i >= len(u) - 1:
            return (-1 - u[i-1][j]) / 2
        else:
            if i <= 0:
                return (u[i+1][j] + 1) / 2
            else:
                return (u[i+1][j] - u[i-1][j]) / 2

    return 0

def second_deriv(u,i,j,first_direction,second_direction):

    # direction == 0 is dx
    # direction == 1 is dy
    if first_direction == 0:
        if second_direction == 1:
            if j < len(u[i]) - 1 and i < len(u) - 1:
                return u[i+1][j+1] - u[i][j+1] - u[i+1][j] + u[i][j]
            else:
                if j >= len(u[i]) - 1 and i >= len(u) - 1:
                    return 1 + u[i][j]
                if j >= len(u[i]) - 1:
                    return - u[i+1][j] + u[i][j]
                if i >= len(u) - 1:
                    return - u[i][j+1] + u[i][j]

        elif second_direction == 0:
            if j > 0 and j < len(u[i]) - 1:
                return u[i][j+1] - 2 * u[i][j] + u[i][j-1]
            else:
                if j <= 0:
                    return u[i][j+1] - 2 * u[i][j] + -1
                if j >= len(u[i]) - 1:
                    return -1 - 2 * u[i][j] + u[i][j-1]

    elif first_direction == 1:
        if second_direction == 0:
            if j < len(u[i]) - 1 and i < len(u) - 1:
                return u[i+1][j+1] - u[i][j+1] - u[i+1][j] + u[i][j]
            else:
                if j >= len(u[i]) - 1 and i >= len(u) - 1:
                    return 1 + u[i][j]
                if i >= len(u) - 1:
                    return - u[i][j+1] + u[i][j]
                if j >= len(u[i]) - 1:
                    return - u[i+1][j] + u[i][j]
        elif second_direction == 1:
            if i > 0 and i < len(u) - 1:
                return u[i+1][j] - 2 * u[i][j] + u[i-1][j]
            else:
                if i <= 0:
                    return u[i+1][j] - 2 * u[i][j] + -1
                if i >= len(u) - 1:
                    return -1 - 2 * u[i][j] + u[i-1][j]

    return 0

def first_deriv(u,i,j,direc,var):

    if var == 0:
        if direc < 0:
            if j > 0:
                return u[i][j] - u[i][j-1]
            if j == 0:
                return u[i][j] + 1.0
        if direc > 0:
            if j < len(u[i]) - 1:
                return u[i][j + 1] - u[i][j]
            if j == len(u[i]) - 1:
                return -1.0 - u[i][j]

    if var == 1:
        if direc < 0:
            if i > 0:
                return - u[i-1][j] + u[i][j]
            if i == 0:
                return u[i][j] + 1.0
        if direc > 0:
            if i <  len(u) - 1:
                return u[i+1][j] - u[i][j]
            if i == len(u) - 1:
                return -1.0 - u[i][j]

    return 0

# Curvature calculation
def curvature(u,i,j,disc_grad,disc_hess):

    denom = ((disc_grad[4][i][j])**2 + (disc_grad[5][i][j])**2)**1.5
    num = disc_hess[0][0][i][j] * (disc_grad[5][i][j])**2 - 2 * disc_grad[5][i][j] * disc_grad[4][i][j] * disc_hess[0][1][i][j] + disc_hess[1][1][i][j] * (disc_grad[4][i][j])**2

    if denom == 0:
        return 0.0

    return num / denom

# For image segmentation functional
def cost_func(i,j,pic):

    return (pic[i][j][0] - 255)**2 - (pic[i][j][0] - 0)**2 + (pic[i][j][1] - 255)**2 - (pic[i][j][1] - 0)**2 + (pic[i][j][2] - 255)**2 - (pic[i][j][2] - 0)**2