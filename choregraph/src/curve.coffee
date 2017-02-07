computeTangent = (dep_c, r_dep, dest_c, r_dest, i) ->
	dep_l = if i % 2 then 1 else -1
	dest_l = if i > 1 then 1 else -1
	is_inner_tan = -dep_l*dest_l
	h = {}
	tan_dep = {}
	tan_dest = {}

	h.x = (r_dep*dest_c.x + is_inner_tan*r_dest*dep_c.x) / (r_dep + is_inner_tan*r_dest)
	h.y = (r_dep*dest_c.y + is_inner_tan*r_dest*dep_c.y) / (r_dep + is_inner_tan*r_dest)

	tan_dep.x = dep_c.x + (r_dep**2*(h.x-dep_c.x) + dep_l*r_dep*(h.y-dep_c.y)*Math.sqrt((h.x-dep_c.x)**2 + (h.y-dep_c.y)**2 - r_dep**2)) /
	((h.x-dep_c.x)**2 + (h.y-dep_c.y)**2)
	tan_dep.y = dep_c.y + (r_dep**2*(h.y-dep_c.y) - dep_l*r_dep*(h.x-dep_c.x)*Math.sqrt((h.x-dep_c.x)**2 + (h.y-dep_c.y)**2 - r_dep**2)) / ((h.x-dep_c.x)**2 + (h.y-dep_c.y)**2)

	tan_dest.x = dest_c.x + (r_dest**2*(h.x-dest_c.x) - is_inner_tan*dest_l*r_dest*(h.y-dest_c.y)*Math.sqrt((h.x-dest_c.x)**2 + (h.y-dest_c.y)**2 - r_dest**2)) / ((h.x-dest_c.x)**2 + (h.y-dest_c.y)**2)
	tan_dest.y = dest_c.y + (r_dest**2*(h.y-dest_c.y) + is_inner_tan*dest_l*r_dest*(h.x-dest_c.x)*Math.sqrt((h.x-dest_c.x)**2 + (h.y-dest_c.y)**2 - r_dest**2)) / ((h.x-dest_c.x)**2 + (h.y-dest_c.y)**2)

	return {dep: tan_dep, dest: tan_dest}

subs = (from, to) -> {x: to.x - from.x, y: to.y - from.y}

det = (a, b) -> a.x*b.y - b.x*a.y

dir2vec = (dir) -> {x: Math.cos(dir), y: Math.sin(dir)}

length = (vec) -> Math.sqrt(vec.x**2 + vec.y**2)

scal = (a, b) -> a.x*b.x + a.y*b.y

sign = (x) -> if x > 0 then 1 else -1

computeCenters = (point, dir, r) ->
	cos = Math.cos(dir)
	sin = Math.sin(dir)
	return [
		{x: point.x + r*sin, y: point.y - r*cos},
		{x: point.x - r*sin, y: point.y + r*cos}
	]

computeInterpoints = (lastMove, move) ->
	r_dep = move.startradius
	r_dest = move.endradius

	dir_dest = move.angle*Math.PI/180
	dir_dep = lastMove.angle*Math.PI/180

	# both circles cannot be of the same radius
	r_dep++ if r_dep == r_dest

	centers_dep = computeCenters(lastMove, dir_dep, r_dep)
	centers_dest = computeCenters(move, dir_dest, r_dest)

	result = {}
	min_dist = 9999999999

	for c_dep in centers_dep
		for c_dest in centers_dest
			det_dep = det(subs(c_dep, lastMove), dir2vec(dir_dep))
			det_dest = det(subs(c_dest, move), dir2vec(dir_dest))
			tan = {}
			for i in [0..3]
				tan = computeTangent(c_dep, r_dep, c_dest, r_dest, i)
				if det(subs(c_dep, tan.dep), subs(tan.dep, tan.dest))*det_dep > 0 and det(subs(c_dest, tan.dest), subs(tan.dep, tan.dest))*det_dest > 0
					break

			if length(subs(tan.dep, tan.dest)) < min_dist
				min_dist = length(subs(tan.dep, tan.dest))
				result =
					center1: c_dep
					center2: c_dest
					end1: tan.dep
					start2: tan.dest

	alpha_dep = Math.abs(2*Math.asin(length(subs(lastMove, result.end1))/(2*r_dep)))
	if scal(subs(lastMove, result.end1), dir2vec(dir_dep)) < 0
		alpha_dep = 2*Math.PI - alpha_dep

	alpha_dest = Math.abs(2*Math.asin(length(subs(move, result.start2))/(2*r_dest)))
	if scal(subs(result.start2, move), dir2vec(dir_dest)) < 0
		alpha_dest = 2*Math.PI - alpha_dest

	result.alpha1 = alpha_dep
	result.alpha2 = alpha_dest

	return result

module.exports = (paper, cm2width, cm2height) ->
	addCenterPointArc = (path, start, center, end, alpha) ->
		middle =
			x: (start.x + end.x)/2
			y: (start.y + end.y)/2
		middleToCenterDist = Math.sqrt((middle.x - center.x)**2 + (middle.y - center.y)**2)
		radius = Math.sqrt((start.x - center.x)**2 + (start.y - center.y)**2)

		if alpha > Math.PI
			radius *= -1

		through =
			x: center.x + (middle.x - center.x)*radius/middleToCenterDist
			y: center.y + (middle.y - center.y)*radius/middleToCenterDist
		try
			path.arcTo(new paper.Point(cm2width(through.x), cm2height(through.y)), new paper.Point(cm2width(end.x), cm2height(end.y)))
		catch
			path.add new paper.Point(cm2width(end.x), cm2height(end.y))

	buildCurve = (path, lastMove, move) ->
		r = computeInterpoints(lastMove, move)

		addCenterPointArc(path, lastMove, r.center1, r.end1, r.alpha1)
		path.add new paper.Point(cm2width(r.start2.x), cm2height(r.start2.y))
		addCenterPointArc(path, r.start2, r.center2, move, r.alpha2)

	return {buildCurve: buildCurve}
