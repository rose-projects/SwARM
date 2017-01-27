computeInterpoints = (lastMove, move) ->
	x_dest = move.x
	y_dest = move.y
	ori_dest = move.angle*Math.PI/180
	x_dep = lastMove.x
	y_dep = lastMove.y
	r_dep = move.startradius
	r_dest = move.endradius

	# system rotation angle
	orientation = lastMove.angle*Math.PI/180
	dep_circle = [0, 0]
	dest_circle = [0, 0]
	# the 2 circles we have to chose from
	tmp1 = [0, 0]
	tmp2 = [0, 0]
	# dirty hack "a la francaise" #duSale
	dirty_hack = 1
	# homothetic center
	h = [0, 0]
	# result points
	pt_tan_dep = []
	pt_tan_dest = []
	dep_l = 0
	dest_l = 0

	# Because of the method of the homothetic center, both circles cannot
	# be of the same radius, this dirty hack solves this issue.
	r_dep++ if r_dep == r_dest

	# choose the right circles
	# hypothesis: the closest to the destination is the one we want.
	# Some cases break this law, but they may not happen depending on the
	# choice of the choreography.
	tmp1[0] = x_dep + r_dep*Math.sin(orientation)
	tmp1[1] = y_dep - r_dep*Math.cos(orientation)
	tmp2[0] = x_dep - r_dep*Math.sin(orientation)
	tmp2[1] = y_dep + r_dep*Math.cos(orientation)
	if (tmp1[0]-x_dest)*(tmp1[0]-x_dest) + (tmp1[1]-y_dest)*(tmp1[1]-y_dest) <= (tmp2[0]-x_dest)*(tmp2[0]-x_dest) + (tmp2[1]-y_dest)*(tmp2[1]-y_dest)
		dep_l = -1
		dep_circle[0] = tmp1[0]
		dep_circle[1] = tmp1[1]
	else
		dep_l = 1
		dep_circle[0] = tmp2[0]
		dep_circle[1] = tmp2[1]

	tmp1[0] = x_dest + r_dest*Math.sin(ori_dest)
	tmp1[1] = y_dest - r_dest*Math.cos(ori_dest)
	tmp2[0] = x_dest - r_dest*Math.sin(ori_dest)
	tmp2[1] = y_dest + r_dest*Math.cos(ori_dest)
	if (tmp1[0]-x_dep)*(tmp1[0]-x_dep) + (tmp1[1]-y_dep)*(tmp1[1]-y_dep) <= (tmp2[0]-x_dep)*(tmp2[0]-x_dep) + (tmp2[1]-y_dep)*(tmp2[1]-y_dep)
		dest_l = -1
		dest_circle[0] = tmp1[0]
		dest_circle[1] = tmp1[1]
	else
		dest_l = 1
		dest_circle[0] = tmp2[0]
		dest_circle[1] = tmp2[1]

	# find the tangent points
	# hypothesis1: the robot will never need to to go to the left if
	# the goal is in the right-hand quadrant, nor behind it
	# hypothesis2: r_dep > r_dest TODO: all cases
	if (dep_l * dest_l == 1)
		is_inner_tan = -1
	else
		is_inner_tan = 1

	dirty_hack = -1 if r_dep < r_dest and is_inner_tan == -1

	h[0] = (r_dep * dest_circle[0] + is_inner_tan * r_dest * dep_circle[0]) / (r_dep + is_inner_tan * r_dest)
	h[1] = (r_dep * dest_circle[1] + is_inner_tan * r_dest * dep_circle[1]) / (r_dep + is_inner_tan * r_dest)

	pt_tan_dep[0] = dep_circle[0] + (r_dep * r_dep * (h[0] - dep_circle[0]) + dirty_hack*dep_l * r_dep * (h[1] - dep_circle[1]) * Math.sqrt((h[0] - dep_circle[0]) * (h[0] - dep_circle[0]) + (h[1] - dep_circle[1]) * (h[1] - dep_circle[1]) - r_dep * r_dep)) / ((h[0] - dep_circle[0]) * (h[0] - dep_circle[0]) + (h[1] - dep_circle[1]) * (h[1] - dep_circle[1]))
	pt_tan_dep[1] = dep_circle[1] + (r_dep * r_dep * (h[1] - dep_circle[1]) - dirty_hack*dep_l * r_dep * (h[0] - dep_circle[0]) * Math.sqrt((h[0] - dep_circle[0]) * (h[0] - dep_circle[0]) + (h[1] - dep_circle[1]) * (h[1] - dep_circle[1]) - r_dep * r_dep)) / ((h[0] - dep_circle[0]) * (h[0] - dep_circle[0]) + (h[1] - dep_circle[1]) * (h[1] - dep_circle[1]))

	pt_tan_dest[0] = dest_circle[0] + (r_dest * r_dest * (h[0] - dest_circle[0]) - dirty_hack*is_inner_tan * dest_l *r_dest * (h[1] - dest_circle[1]) * Math.sqrt((h[0] - dest_circle[0]) * (h[0] - dest_circle[0]) + (h[1] - dest_circle[1]) * (h[1] - dest_circle[1]) - r_dest * r_dest)) / ((h[0] - dest_circle[0]) * (h[0] - dest_circle[0]) + (h[1] - dest_circle[1]) * (h[1] - dest_circle[1]))
	pt_tan_dest[1] = dest_circle[1] + (r_dest * r_dest * (h[1] - dest_circle[1]) + dirty_hack*is_inner_tan * dest_l * r_dest * (h[0] - dest_circle[0]) * Math.sqrt((h[0] - dest_circle[0]) * (h[0] - dest_circle[0]) + (h[1] - dest_circle[1]) * (h[1] - dest_circle[1]) - r_dest * r_dest)) / ((h[0] - dest_circle[0]) * (h[0] - dest_circle[0]) + (h[1] - dest_circle[1]) * (h[1] - dest_circle[1]))

	return {
		center1:
			x: dep_circle[0]
			y: dep_circle[1]
		center2:
			x: dest_circle[0]
			y: dest_circle[1]
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
			if dir.x*Math.cos(angle*Math.PI/180) + dir.y*Math.sin(angle*Math.PI/180) < 0
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
