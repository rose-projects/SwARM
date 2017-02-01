computeInterpoints = (lastMove, move) ->
	x_dep = lastMove.x
	y_dep = lastMove.y
	x_dest = move.x
	y_dest = move.y
	r_dep = move.startradius
	r_dest = move.endradius
	ori_dest = move.angle*Math.PI/180

	orientation = lastMove.angle*Math.PI/180 # orientation of the robot
	# center of the departure and destination circles
	dep_c = [0, 0]
	dest_c = [0, 0]
	# the circles we have to chose from
	tmp1 = [0, 0]
	tmp2 = [0, 0]
	tmp3 = [0, 0]
	tmp4 = [0, 0]
	dirty_hack = 1 # dirty hack to fix the algo when the r_dep < r_dest
	h = [0, 0] # homothetic center
	# tangent points
	pt_tan_dep = []
	pt_tan_dest = []
	# 1 if to the left, -1 otherwise
	dep_l = 0
	dest_l = 0
	dep_cos = 0
	dep_sin = 0
	dest_cos = 0
	dest_sin = 0

	# Because of the method of the homothetic center, both circles cannot
	# be of the same radius
	r_dep++ if r_dep == r_dest

	# choose the right circles
	# First, pick the closest circle. If an error is detected after the
	# tangents are computed, correct it.
	dep_cos = Math.cos(orientation)
	dep_sin = Math.sin(orientation)
	tmp1[0] = x_dep + r_dep*dep_sin
	tmp1[1] = y_dep - r_dep*dep_cos
	tmp2[0] = x_dep - r_dep*dep_sin
	tmp2[1] = y_dep + r_dep*dep_cos
	if (tmp1[0]-x_dest)**2 + (tmp1[1]-y_dest)**2 <= (tmp2[0]-x_dest)**2 + (tmp2[1]-y_dest)**2
		dep_l = -1
		dep_c[0] = tmp1[0]
		dep_c[1] = tmp1[1]
	else
		dep_l = 1
		dep_c[0] = tmp2[0]
		dep_c[1] = tmp2[1]

	dest_cos = Math.cos(ori_dest)
	dest_sin = Math.sin(ori_dest)
	tmp3[0] = x_dest + r_dest*dest_sin
	tmp3[1] = y_dest - r_dest*dest_cos
	tmp4[0] = x_dest - r_dest*dest_sin
	tmp4[1] = y_dest + r_dest*dest_cos
	if (tmp3[0]-x_dep)**2 + (tmp3[1]-y_dep)**2 <= (tmp4[0]-x_dep)**2 + (tmp4[1]-y_dep)**2
		dest_l = -1
		dest_c[0] = tmp3[0]
		dest_c[1] = tmp3[1]
	else
		dest_l = 1
		dest_c[0] = tmp4[0]
		dest_c[1] = tmp4[1]

	# find the tangent points
	# hypothesis1: the robot will never need to to go to the left if
	# the goal is in the right-hand quadrant, nor behind it
	for i in [0..1]
		if dep_l*dest_l == 1
			is_inner_tan = -1
		else
			is_inner_tan = 1
	
		dirty_hack = -1 if r_dep < r_dest and is_inner_tan == -1
	
		h[0] = (r_dep*dest_c[0] + is_inner_tan*r_dest*dep_c[0]) / (r_dep + is_inner_tan*r_dest)
		h[1] = (r_dep*dest_c[1] + is_inner_tan*r_dest*dep_c[1]) / (r_dep + is_inner_tan*r_dest)
	
		pt_tan_dep[0] = dep_c[0] + (r_dep**2*(h[0]-dep_c[0]) + dirty_hack*dep_l*r_dep*(h[1]-dep_c[1])*Math.sqrt((h[0]-dep_c[0])**2 + (h[1]-dep_c[1])**2 - r_dep**2)) / ((h[0]-dep_c[0])**2 + (h[1]-dep_c[1])**2)
		pt_tan_dep[1] = dep_c[1] + (r_dep**2*(h[1]-dep_c[1]) - dirty_hack*dep_l*r_dep*(h[0]-dep_c[0])*Math.sqrt((h[0]-dep_c[0])**2 + (h[1]-dep_c[1])**2 - r_dep**2)) / ((h[0]-dep_c[0])**2 + (h[1]-dep_c[1])**2)
	
		pt_tan_dest[0] = dest_c[0] + (r_dest**2*(h[0]-dest_c[0]) - dirty_hack*is_inner_tan*dest_l*r_dest*(h[1]-dest_c[1])*Math.sqrt((h[0]-dest_c[0])**2 + (h[1]-dest_c[1])**2 - r_dest**2)) / ((h[0]-dest_c[0])**2 + (h[1]-dest_c[1])**2)
		pt_tan_dest[1] = dest_c[1] + (r_dest**2*(h[1]-dest_c[1]) + dirty_hack*is_inner_tan*dest_l*r_dest*(h[0]-dest_c[0])*Math.sqrt((h[0]-dest_c[0])**2 + (h[1]-dest_c[1])**2 - r_dest**2)) / ((h[0]-dest_c[0])**2 + (h[1]-dest_c[1])**2)

		# Correct the error if the wrong circle was chosen. Happens when
		# the direction goes through the opposite circle.
		if (pt_tan_dest[0]-pt_tan_dep[0])*dep_cos + (pt_tan_dest[1]-pt_tan_dep[1])*dep_sin > 0 and (pt_tan_dep[0]-x_dep)*dep_cos + (pt_tan_dep[1]-y_dep)*dep_sin < 0
			dep_l *= -1
			if dep_c[0] == tmp1[0] and dep_c[1] == tmp1[1]
				dep_c[0] = tmp2[0]
				dep_c[1] = tmp2[1]
			else
				dep_c[0] = tmp1[0]
				dep_c[1] = tmp1[1]
			continue

		if (pt_tan_dest[0]-pt_tan_dep[0])*dest_cos + (pt_tan_dest[1]-pt_tan_dep[1])*dest_sin > 0 and (x_dest-pt_tan_dest[0])*dest_cos + (y_dest-pt_tan_dest[1])*dest_sin < 0
			dest_l *= -1
			if dest_c[0] == tmp3[0] and dest_c[0] == tmp3[0]
				dest_c[0] = tmp4[0]
				dest_c[1] = tmp4[1]
			else
				dest_c[0] = tmp3[0]
				dest_c[1] = tmp3[1]
			continue
		break	

	return {
		center1:
			x: dep_c[0]
			y: dep_c[1]
		center2:
			x: dest_c[0]
			y: dest_c[1]
		end1:
			x: pt_tan_dep[0]
			y: pt_tan_dep[1]
		start2:
			x: pt_tan_dest[0]
			y: pt_tan_dest[1]
	}

module.exports = (paper, cm2width, cm2height) ->
	addCenterPointArc = (path, start, center, end, angle, dir) ->
		middle =
			x: (start.x + end.x)/2
			y: (start.y + end.y)/2
		middleToCenterDist = Math.sqrt((middle.x - center.x)**2 + (middle.y - center.y)**2)
		radius = Math.sqrt((start.x - center.x)**2 + (start.y - center.y)**2)

		if (end.x - start.x)*Math.cos(angle*Math.PI/180) + (end.y - start.y)*Math.sin(angle*Math.PI/180) < 0
			radius *= -1

		through =
			x: center.x + (middle.x - center.x)*radius/middleToCenterDist
			y: center.y + (middle.y - center.y)*radius/middleToCenterDist

		path.arcTo(new paper.Point(cm2width(through.x), cm2height(through.y)), new paper.Point(cm2width(end.x), cm2height(end.y)))

	buildCurve = (path, lastMove, move) ->
		r = computeInterpoints(lastMove, move)
		dir =
			x: r.start2.x - r.end1.x
			y: r.start2.y - r.end1.y
		addCenterPointArc(path, lastMove, r.center1, r.end1, lastMove.angle, dir)
		path.add new paper.Point(cm2width(r.start2.x), cm2height(r.start2.y))
		addCenterPointArc(path, r.start2, r.center2, move, move.angle, dir)

	return {buildCurve: buildCurve}
