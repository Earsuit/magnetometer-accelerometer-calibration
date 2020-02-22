function EllipsoidFitting(raw)
%   only selet the x,y,z values for acceleration or magnet
    format long
    raw = table2array(raw);
    tmp = size(raw);
    psi = [raw(:,2).^2 raw(:,3).^2 raw(:,1) raw(:,2) raw(:,3) ones(tmp(1,1),1)];
    Y = -(raw(:,1).^2);
    P = inv(transpose(psi)*psi)
    theta = P*transpose(psi)*Y
    x_c = theta(3,1)/-2;
    y_c = theta(4,1)/(-2*theta(1,1));
    z_c = theta(5,1)/(-2*theta(2,1));
    x_r = sqrt(x_c^2+theta(1,1)*y_c^2+theta(2,1)*z_c^2-theta(6,1));
    y_r = sqrt(x_r^2/theta(1,1));
    z_r = sqrt(x_r^2/theta(2,1));
    ret = [x_c;y_c;z_c;x_r;y_r;z_r]
%   draw the ellipsoid and the raw data points
    [x,y,z] = ellipsoid(x_c,y_c,z_c,x_r,y_r,z_r);
    surf(x, y, z)
    hold on
    alpha 0.2
    plot3(raw(:,1),raw(:,2),raw(:,3),'.')
end