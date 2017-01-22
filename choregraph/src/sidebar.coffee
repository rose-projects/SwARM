$ = require 'jquery'
list = require '../lib/list'

mode = 'moves'
robots = []
robotList = list('robots', robots, "ajouter un robot", (i) -> "Robot #{i}")

robotList.onAdd (item) ->
	item.moves = []
	item.colors = []
	item.movesList = list('robots-content', item.moves, 'ajouter un point', (i) -> "Point #{i}")
	item.colorsList = list('robots-content', item.colors, 'ajouter une couleur', (i) -> "Couleur #{i}")

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
