computeInterpoints = (lastMove, move) ->
	x_goal = move.x
	y_goal = move.y
	goal_angle = move.angle*180/Math.PI
	x_pos = lastMove.x
	y_pos = lastMove.y
	r_dep = move.startradius
	r_goal = move.endradius

	# system rotation angle
	theta = Math.PI/2 - lastMove.angle*180/Math.PI
	dep_circle = [0, 0]
	goal_circle = [0, 0]
	# the 2 arrival circles we have to chose from
	tmp1 = [0, 0]
	tmp2 = [0, 0]
	# homothetic center
	h = [0, 0]

	# Because of the method of the homothetic center, both circles cannot
	# be of the same radius, this dirty hack solves this issue.
	r_dep++ if r_dep == r_goal

	# cartesian system change
	x_goal_ = (x_goal-x_pos)*Math.cos(theta) - (y_goal-y_pos)*Math.sin(theta)
	y_goal_ = (x_goal-x_pos)*Math.sin(theta) + (y_goal-y_pos)*Math.cos(theta)
	goal_angle_ = goal_angle + theta

	# choose the right circles
	# hypothesis: the closest to the goal is the one we want.
	# Some cases break this law, but they may not happen depending on the
	# choice of the choreography.
	dep_circle[0] = if x_goal_ >= 0 then r_dep else -r_dep
	tmp1[0] = x_goal_ + r_goal * Math.sin(goal_angle_)
	tmp1[1] = y_goal_ - r_goal * Math.cos(goal_angle_)
	tmp2[0] = x_goal_ - r_goal * Math.sin(goal_angle_)
	tmp2[1] = y_goal_ + r_goal * Math.cos(goal_angle_)
	if tmp1[0]*tmp1[0] + tmp1[1]*tmp1[1] <= tmp2[0]*tmp2[0] + tmp2[1]*tmp2[1]
		goal_circle[0] = tmp1[0]
		goal_circle[1] = tmp1[1]

	# find the tangent points
	# hypothesis1: the robot will never need to to go to the left if
	# the goal is in the right-hand quadrant
	# hypothesis2: r_dep > r_goal TODO: all cases
	if goal_circle[0] == tmp1[0]
		t = -1
	else
		t = 1

	h[0] = (r_dep*dep_circle[0] + t*r_goal*goal_circle[0]) / (r_dep + t*r_goal)
	h[1] = (r_dep*dep_circle[1] + t*r_goal*goal_circle[1]) / (r_dep + t*r_goal)

	pt_tan_dep = []
	pt_tan_goal = []
	pt_tan_dep[0] = dep_circle[0] + (r_dep*r_dep*(h[0]-dep_circle[0]) - r_dep*(h[1]-dep_circle[1]) * Math.sqrt((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) + (h[1]-dep_circle[1])*(h[1]-dep_circle[1]) - r_dep*r_dep)) / ((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) + (h[1]-dep_circle[1])*(h[1]-dep_circle[1]))
	pt_tan_dep[1] = dep_circle[1] + (r_dep*r_dep*(h[1]-dep_circle[1]) + r_dep*(h[0]-dep_circle[0]) * Math.sqrt((h[0]-dep_circle[0])*(h[0] -  dep_circle[0]) + (h[1]-dep_circle[1])*(h[1]-dep_circle[1]) - r_dep*r_dep)) / ((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) + (h[1]-dep_circle[1])*(h[1]-dep_circle[1]))
	pt_tan_goal[0] = goal_circle[0] + (r_goal*r_goal*(h[0]-goal_circle[0]) + t*r_goal*(h[1]-goal_circle[0]) * Math.sqrt((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) + (h[1]-goal_circle[1])*(h[1]-goal_circle[1]) - r_goal*r_goal)) / ((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) + (h[1]-goal_circle[1])*(h[1]-goal_circle[1]))
	pt_tan_goal[1] = goal_circle[1] + (r_goal*r_goal*(h[1]-goal_circle[1]) - t*r_goal*(h[0]-goal_circle[0]) * Math.sqrt((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) + (h[1]-goal_circle[1])*(h[1]-goal_circle[1]) - r_goal*r_goal)) / ((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) + (h[1]-goal_circle[1])*(h[1]-goal_circle[1]))

	return {
		pt1:
			x: x_pos + (x_goal - x_pos)*1/4
			y: y_pos + (y_goal - y_pos)*1/4
		pt2:
			x: x_pos + (x_goal - x_pos)*3/4
			y: y_pos + (y_goal - y_pos)*3/4
	}

module.exports = (paper, cm2width, cm2height) ->
	buildCurve = (path, lastMove, move) ->
		r = computeInterpoints(lastMove, move)
		path.add new paper.Point(cm2width(r.pt1.x), cm2height(r.pt1.y))
		path.add new paper.Point(cm2width(r.pt2.x), cm2height(r.pt2.y))
		path.add new paper.Point(cm2width(move.x), cm2height(move.y))

	return {buildCurve: buildCurve}
