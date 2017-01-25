paper = require '../ressources/scripts/paper-core.min.js'
$ = require 'jquery'

cm2width = (value) -> value*paper.view.size.width/800
cm2height = (value) -> (500-value)*paper.view.size.height/500
width2cm = (value) -> value*800/paper.view.size.width
height2cm = (value) -> 500 - value*500/paper.view.size.height
vec2angle = (vec) ->
	rad = Math.atan2(vec.x, vec.y)
	return Math.round(450-rad*180/Math.PI) % 360
hsv2rgb = (h, s, v) ->
	rgb = []

	h = h*360/255
	s = s/255
	v = v/255

	if s == 0
		rgb = [v,v,v];
	else
		h = h / 60;
		i = Math.floor(h);
		data = [v*(1-s), v*(1-s*(h-i)), v*(1-s*(1-(h-i)))];
		switch(i)
			when 0 then rgb = [v, data[2], data[0]]
			when 1 then rgb = [data[1], v, data[0]]
			when 2 then rgb = [data[0], v, data[2]]
			when 3 then rgb = [data[0], data[1], v]
			when 4 then rgb = [data[2], data[0], v]
			else rgb = [v, data[0], data[1]]
	return "rgb(#{rgb[0]*255}, #{rgb[1]*255}, #{rgb[2]*255})"

curve = require('../lib/curve')(paper, cm2width, cm2height)

robotPaths = []
ignore = false
robotList = null
mode = 'moves'

robotPath = (robot) ->
	path = null
	points = []
	colors = []
	skirts = []
	simRobot = null

	clear = ->
		path?.remove()
		pt.remove() for pt in points
		cl.remove() for cl in colors
		sk.remove() for sk in skirts

	getPointIndex = (x, y) ->
		ref = new paper.Point(x, y)
		for pt in points
			return i if pt.getDistance(ref) < 1
		return -1

	createPoint = (move) ->
		pt = new paper.Point(cm2width(move.x), cm2height(move.y))

		skirt = new paper.Path.Circle pt, cm2width(14)
		skirt.fillColor = "black"
		skirt.opacity = 0

		point = new paper.Path.Circle pt, cm2width(4)
		point.fillColor = "black"
		point.strokeColor = "black"
		point.segments[2].point = new paper.Point(cm2width(move.x + 8), cm2height(move.y));
		point.segments[2].handleIn = new paper.Point(0, 0);
		point.segments[2].handleOut = new paper.Point(0, 0);
		point.rotate(move.angle, pt)

		rb = new paper.Path.Circle pt, cm2width(35)
		rb.strokeColor = "black"
		rb.visible = no

		point.onMouseEnter = (event) ->
			rb.visible = yes
			document.body.style.cursor = "crosshair"
			this.fillColor = "red"
		point.onMouseLeave = (event) ->
			document.body.style.cursor = "default"
			this.fillColor = "black"
			rb.visible = no
		point.onMouseDrag = (event) ->
			move.x = Math.round(width2cm(event.point.x))
			move.y = Math.round(height2cm(event.point.y))
			rb.remove()
			robot.movesList.activate(move.index)
			update()
			event.stop()
		point.onMouseDown = (event) ->
			robot.movesList.activate(move.index)
			if paper.Key.isDown('x')
				rb.remove()
				removePoint(move.index)
			ignore = true

		skirt.onMouseEnter = -> rb.visible = yes
		skirt.onMouseDown = -> ignore = true
		skirt.onMouseDrag = (event) ->
			move.angle = vec2angle({x: event.point.x - pt.x, y: event.point.y - pt.y})
			rb.remove()
			robot.movesList.activate(move.index)
			update()
		skirt.onMouseLeave = -> rb.visible = false

		points.push(point)
		skirts.push(skirt)
	createColor = (color) ->
		return if robot.moves.length == 0

		index = -1
		pt = null
		for mv in robot.moves when mv.date <= color.date
			index = mv.index
		if index == robot.moves.length - 1
			pt = new paper.Point(cm2width(robot.moves[index].x), cm2height(robot.moves[index].y))
		else if index == -1
			pt = new paper.Point(cm2width(robot.moves[0].x), cm2height(robot.moves[0].y))
		else
			start = path.getOffsetOf(new paper.Point(cm2width(robot.moves[index].x), cm2height(robot.moves[index].y)))
			end = path.getOffsetOf(new paper.Point(cm2width(robot.moves[index+1].x), cm2height(robot.moves[index+1].y)))
			startDate = robot.moves[index].date
			endDate = robot.moves[index+1].date
			offset = 0
			if startDate == endDate
				offset = start
			else
				offset = start + (end - start)/(endDate - startDate)*(color.date-startDate)
			pt = path.getPointAt offset

		radius = if color.activated then 10 else 6
		point = new paper.Path.Circle pt, cm2width(radius)
		point.fillColor = color.rgb
		colors.push point
	showRobotAt = (date) ->
		return if robot.moves.length == 0

		# compute location
		index = -1
		pt = null
		for mv in robot.moves when mv.date <= date
			index = mv.index
		if index == robot.moves.length - 1
			pt = new paper.Point(cm2width(robot.moves[index].x), cm2height(robot.moves[index].y))
		else if index == -1
			pt = new paper.Point(cm2width(robot.moves[0].x), cm2height(robot.moves[0].y))
		else
			start = path.getOffsetOf(new paper.Point(cm2width(robot.moves[index].x), cm2height(robot.moves[index].y)))
			end = path.getOffsetOf(new paper.Point(cm2width(robot.moves[index+1].x), cm2height(robot.moves[index+1].y)))
			startDate = robot.moves[index].date
			endDate = robot.moves[index+1].date
			offset = 0
			if startDate == endDate
				offset = start
			else
				offset = start + (end - start)/(endDate - startDate)*(date-startDate)
			pt = path.getPointAt offset

		# compute color
		color = ""
		index = -1
		for cl in robot.colors when cl.date <= date
			index = cl.index
		if index == -1 or robot.colors.length == 0
			color = "black"
		else if index == robot.colors.length - 1
			color = robot.colors[index].rgb
		else
			# if we're fading
			if robot.colors[index + 1].date - robot.colors[index + 1].fade < date
				next = robot.colors[index + 1]
				current = robot.colors[index]
				h = current.h + (next.h - current.h)*(date - (next.date - next.fade))/next.fade
				s = current.s + (next.s - current.s)*(date - (next.date - next.fade))/next.fade
				v = current.v + (next.v - current.v)*(date - (next.date - next.fade))/next.fade
				console.log current.h, next.h, date, next.fade, next.date
				color = hsv2rgb(h, s, v)
			else
				color = robot.colors[index].rgb

		simRobot.remove() if simRobot?
		point = new paper.Path.Circle pt, cm2width(35)
		point.fillColor = color
		simRobot = point

	update = ->
		clear()
		if robot.selected
			path = new paper.Path()
			path.strokeWidth = 2
			path.strokeColor = "black"
			if robot.activated and mode is 'moves'
				createPoint(move) for move in robot.moves when move.selected

			lastMove = null
			for move in robot.moves
				if move.selected
					if lastMove?
						curve.buildCurve(path, lastMove, move)
					else
						path.add new paper.Point(cm2width(move.x), cm2height(move.y))
					lastMove = move
				else
					lastMove = null
			if robot.activated and mode is 'colors'
				createColor(color) for color in robot.colors when color.selected
	removePoint = (x, y) ->
		if typeof y is 'undefined'
			index = x
		else
			index = getPointIndex(x, y)

		if(index > -1)
			robot.movesList.remove(index)
			update()

	addPoint = (point, insert) ->
		if insert
			index = Math.floor(path.getNearestLocation(point).index/3) + 1
			console.log index
			robot.movesList.add(index)
		else
			index = robot.moves.length
			robot.movesList.add()

		if robot.moves.length > 1
			robot.moves[index].date = robot.moves[index - 1].date
		robot.moves[index].x = Math.round(width2cm(point.x))
		robot.moves[index].y = Math.round(height2cm(point.y))

		robot.movesList.update()
		update()

	update()

	return {
		addPoint: addPoint
		removePoint: removePoint
		update: update

		distance: (point) ->
			if points.length < 2 or not path.getNearestLocation(point)?
				return 20000
			nearest = path.getNearestLocation(point).point
			return width2cm( Math.sqrt((nearest.x - point.x)**2 + (nearest.y - point.y)**2) )
		remove: ->
			clear()
			robotPaths.splice(robot.index, 1)
		activate : ->
			robotList.activate(robot.index)
			update()
		visible: (vis) ->
			robot.selected = vis
			update()
		activated: -> robot.activated
		isVisible: -> robot.selected
		showSimAt: showRobotAt
		hideSim: -> simRobot?.remove()
	}

setup = (list) ->
	robotList = list
	paper.setup document.getElementById('table')

	paper.view.onMouseDown = (event) ->
		# event has already been processed
		if ignore or paper.Key.isDown('x')
			ignore = false;
			return;

		# activate path if user clicks on it
		for path in robotPaths when path.isVisible() and not path.activated() and path.distance(event.point) < 10
			path.activate()
			return

		# if a path is active
		if robotList.getActive()? and mode is 'moves'
			active = robotPaths[robotList.getActive().index]
			# A + click on path => split a segment
			if paper.Key.isDown('a') && active.distance(event.point) < 5
				active?.addPoint(event.point, yes)
			else
				active?.addPoint(event.point, no)

module.exports = {
	setup: setup
	setMode: (md) ->
		mode = md
		path.update() for path in robotPaths
	updatePaths: path.update() for path in robotPaths
	addRobot: (robot) ->
		robotPaths.push(robotPath(robot))
	paths: -> robotPaths
}
