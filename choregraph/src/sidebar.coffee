$ = require 'jquery'
list = require '../lib/list'
jade = require 'jade'
jsonfile = require 'jsonfile'
ipc = require('electron').ipcRenderer
app = require('electron').remote.app

createMoveList = (item) ->
	l = list('robots-content', item.moves, 'ajouter un point', (i) -> "Point #{i}")
	l.onAdd (item) ->
		item.date = 0
		item.x = 100
		item.y = 100
		item.angle = 0
		item.startradius = 0
		item.endradius = 20
	l.onUpdate (item) ->
		$('.robots-content-content').html jade.renderFile __dirname + '/../views/move.jade', move: item
		$('#date').change ->
			val = parseInt($('#date').val())
			item.date = val unless isNaN(val)
		$('#x-goal').change ->
			val = parseInt($('#x-goal').val())
			item.x = val unless isNaN(val)
		$('#y-goal').change ->
			val = parseInt($('#y-goal').val())
			item.y = val unless isNaN(val)
		$('#angle').change ->
			val = parseInt($('#angle').val())
			item.angle = val unless isNaN(val)
		$('#start-radius').change ->
			val = parseInt($('#start-radius').val())
			item.startradius = val unless isNaN(val)
		$('#end-radius').change ->
			val = parseInt($('#end-radius').val())
			item.endradius = val unless isNaN(val)
	return l

createColorList = (item) ->
	l = list('robots-content', item.colors, 'ajouter une couleur', (i) -> "Couleur #{i}")
	l.onAdd (item) ->
		item.date = 0
		item.h = 0
		item.s = 0
		item.v = 0
		item.fade = 1
	l.onUpdate (item) ->
		$('.robots-content-content').html jade.renderFile __dirname + '/../views/color.jade', color: item
		picker = new window.jscolor(document.getElementById('color'))
		picker.fromHSV(item.h*360/255, item.s*100/255, item.v*100/255)

		$('#date').change ->
			val = parseInt($('#date').val())
			item.date = val unless isNaN(val)
		$('#fade').change ->
			val = parseInt($('#fade').val())
			item.fade = val unless isNaN(val)
		$('#color').change ->
			item.h = Math.floor(picker.hsv[0]*255/360)
			item.s = Math.floor(picker.hsv[1]*255/100)
			item.v = Math.floor(picker.hsv[2]*255/100)
	return l

mode = 'moves'
try
	dance = jsonfile.readFileSync __dirname+'/../dance.json'
robots = if dance?.robots? then dance.robots else []
robotList = list('robots', robots, "ajouter un robot", (i) -> "Robot #{i}")

for robot in robots
	robot.movesList = createMoveList(robot)
	robot.colorsList = createColorList(robot)

robotList.onAdd (item) ->
	item.moves = []
	item.colors = []
	item.movesList = createMoveList(item)
	item.colorsList = createColorList(item)

robotList.onUpdate (item) ->
	if mode is 'moves'
		item.movesList.update()
	else
		item.colorsList.update()

robotList.onRemove -> console.log "removed"
robotList.onActivated -> console.log "activated"
robotList.onDesactivated -> console.log "desactivated"
robotList.onSelected -> console.log "selected"
robotList.onDeselected -> console.log "deselected"

$ ->
	$('.colors-mode').click ->
		if mode isnt 'colors'
			mode = 'colors'
			$('.moves-mode').removeClass('selected-btn')
			$(this).addClass('selected-btn')
			robotList.update()

	$('.moves-mode').click ->
		if mode isnt 'moves'
			mode = 'moves'
			$('.colors-mode').removeClass('selected-btn')
			$(this).addClass('selected-btn')
			robotList.update()

	robotList.update()

ipc.on 'closing', ->
	jsonfile.writeFileSync __dirname+'/../dance.json', robots: robots
	app.exit(0)
