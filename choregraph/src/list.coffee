jade = require 'jade'
$ = require 'jquery'

module.exports = (className, data, addMsg, nameGenerator) ->
	# callbacks
	onUpdate = null
	onAdd = null
	onRemove = null
	onActivated = null
	onDesactivated = null
	onSelected = null
	onDeselected = null

	getActive = ->
		for item in data when item.activated
			return item

	# update list in the DOM
	update = ->
		$(".#{className}").html jade.renderFile __dirname + '/../views/list.jade',
			addMsg: addMsg
			list: data
			listname: className

		$(".#{className + '-name'}").click ->
			index = parseInt($(this).parent().data("index"));
			activate(index) if not data[index].activated
		$(".#{className + '-delete'}").click ->
			index = parseInt($(this).parent().parent().data("index"));
			remove(index)
		$(".#{className + '-checkbox'}").change ->
			index = parseInt($(this).parent().parent().data("index"));
			if $(this).is(':checked')
				select(index, no)
			else
				deselect(index, no)
		$(".#{className + '-add'}").click add

		onUpdate?(getActive())

	# add an element
	add = ->
		data.push {
			name: nameGenerator(data.length + 1)
			index: data.length
			selected: yes
			activated: no
		}
		onAdd?(data[data.length - 1])
		activate(data.length - 1)
	# remove an element
	remove = (index) ->
		onRemove?(data[index])
		data.splice(index, 1)
		for i in [0..data.length - 1]
			data[i].index = i
			data[i].name = nameGenerator i + 1
		activate(data.length - 1)

	# when a list item is clicked on
	activate = (index) ->
		for item in data when item.activated
			item.activated = no
			onDesactivated?(item)
		data[index]?.activated = yes
		update()
		onActivated?(data[index])

	# when the item checkbox is activated
	select = (index, mustUpdate) ->
		data[index]?.selected = yes
		update() if mustUpdate
		onSelected?(data[index])
	# when the item checkbox is desactivated
	deselect = (index, mustUpdate) ->
		data[index]?.selected = no
		update() if mustUpdate
		onDeselected?(data[index])

	update()

	return {
		add: add
		remove: remove
		getActive: getActive
		activate: activate
		select: (index) -> select(index, yes)
		deselect: (index) -> deselect(index, yes)
		update: update
		onUpdate: (callback) -> onUpdate = callback
		onAdd: (callback) -> onAdd = callback
		onRemove: (callback) -> onRemove = callback
		onActivated: (callback) -> onActivated = callback
		onDesactivated: (callback) -> onDesactivated = callback
		onSelected: (callback) -> onSelected = callback
		onDeselected: (callback) -> onDeselected = callback
	}
