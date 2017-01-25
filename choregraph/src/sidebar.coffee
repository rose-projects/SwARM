$ = require 'jquery'
list = require '../lib/list'
path = require '../lib/path'
simu = require '../lib/simu'
jade = require 'jade'
jsonfile = require 'jsonfile'
ipc = require('electron').ipcRenderer
app = require('electron').remote.app

createMoveList = (rb) ->
	l = list('robots-content', rb.moves, 'ajouter un point', (i) -> "Point #{i}")
	l.onAdd (item) ->
		item.date = 0
		item.x = 100
		item.y = 100
		item.angle = 0
		item.startradius = 20
		item.endradius = 20
		path.paths()[rb.index].update()
	l.onUpdate (item) ->
		$('.robots-content-content').html jade.renderFile __dirname + '/../views/move.jade', move: item
		$('#date').change ->
			val = parseInt($('#date').val())
			item.date = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#x-goal').change ->
			val = parseInt($('#x-goal').val())
			item.x = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#y-goal').change ->
			val = parseInt($('#y-goal').val())
			item.y = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#angle').change ->
			val = parseInt($('#angle').val())
			item.angle = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#start-radius').change ->
			val = parseInt($('#start-radius').val())
			item.startradius = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#end-radius').change ->
			val = parseInt($('#end-radius').val())
			item.endradius = val unless isNaN(val)
			path.paths()[rb.index].update()
	l.onRemove ->
		path.paths()[rb.index].update()
	l.onActivated(-> path.paths()[rb.index].update())
	l.onDesactivated(-> path.paths()[rb.index].update())
	l.onSelected(-> path.paths()[rb.index].update())
	l.onDeselected(-> path.paths()[rb.index].update())
	return l

createColorList = (rb) ->
	l = list('robots-content', rb.colors, 'ajouter une couleur', (i) -> "Couleur #{i}")
	l.onAdd (item) ->
		item.date = 0
		item.h = 0
		item.s = 0
		item.v = 0
		item.rgb = "rgb(0, 0, 0)"
		item.fade = 1
		path.paths()[rb.index].update()
	l.onUpdate (item) ->
		$('.robots-content-content').html jade.renderFile __dirname + '/../views/color.jade', color: item
		picker = new window.jscolor(document.getElementById('color'))
		picker.fromHSV(item.h*360/255, item.s*100/255, item.v*100/255)

		$('#date').change ->
			val = parseInt($('#date').val())
			item.date = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#fade').change ->
			val = parseInt($('#fade').val())
			item.fade = val unless isNaN(val)
			path.paths()[rb.index].update()
		$('#color').change ->
			item.h = Math.floor(picker.hsv[0]*255/360)
			item.s = Math.floor(picker.hsv[1]*255/100)
			item.v = Math.floor(picker.hsv[2]*255/100)
			item.rgb = picker.toRGBString()
			path.paths()[rb.index].update()
	l.onRemove(-> path.paths()[rb.index].update())
	l.onActivated(-> path.paths()[rb.index].update())
	l.onDesactivated(-> path.paths()[rb.index].update())
	l.onSelected(-> path.paths()[rb.index].update())
	l.onDeselected(-> path.paths()[rb.index].update())
	return l

mode = 'moves'
try
	dance = jsonfile.readFileSync __dirname+'/../dance.json'
robots = if dance?.robots? then dance.robots else []
robotList = list('robots', robots, "ajouter un robot", (i) -> "Robot #{i}")

robotList.onAdd (item) ->
	item.moves = []
	item.colors = []

	path.addRobot item
	item.movesList = createMoveList(item)
	item.colorsList = createColorList(item)

robotList.onUpdate (item) ->
	if mode is 'moves'
		item.movesList.update()
	else
		item.colorsList.update()
	path.paths()[item.index].update()

robotList.onRemove((item) -> path.paths()[item.index].remove())
robotList.onActivated((item) -> path.paths()[item.index].update())
robotList.onDesactivated((item) -> path.paths()[item.index].update())
robotList.onSelected((item) -> path.paths()[item.index].update())
robotList.onDeselected((item) -> path.paths()[item.index].update())

$ ->
	$('.colors-mode').click ->
		if mode isnt 'colors'
			mode = 'colors'
			$('.moves-mode').removeClass('selected-btn')
			$(this).addClass('selected-btn')
			robotList.update()
			path.setMode 'colors'
	$('.moves-mode').click ->
		if mode isnt 'moves'
			mode = 'moves'
			$('.colors-mode').removeClass('selected-btn')
			$(this).addClass('selected-btn')
			robotList.update()
			path.setMode 'moves'

	path.setup(robotList)

	for robot in robots
		robot.movesList = createMoveList(robot)
		robot.colorsList = createColorList(robot)
		path.addRobot robot

	robotList.update()
	simu(path.paths())

ipc.on 'closing', ->
	jsonfile.writeFileSync __dirname+'/../dance.json', robots: robots
	app.exit(0)
