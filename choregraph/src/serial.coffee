serialport = require 'serialport'
$ = require 'jquery'

port = null
connected = no
onConnected = null

intRead = (cl, def) ->
	val = parseInt $(".#{cl}").val()
	return if isNaN(val) then def else val

flashRobot = (id, robot) ->
	console.log 'flashing robot', robot.index, 'to ID', id
	$('.flash-console').show().text('')
	$('.darkscreen').show()
		.click ->
			$('.darkscreen').hide()
			$('.flash-console').hide()

module.exports = ->
	$ ->
		$('.darkscreen').hide()
		$('.flash-console').hide()

		serialport.list (err, ports) ->
			return if err?
			for p in ports.reverse()
				$('.serialpath').append "<option value=\"#{p.comName}\">#{p.comName}</option>"

		$('.serial-btn').click ->
			connected = no
			port.close() if port? and port?.isOpen()
			port = new serialport $('.serialpath').val(),
				baudRate: intRead('baudrate', 38400)
				parser: serialport.parsers.readline('\n')

			port.on 'open', ->
				connected = yes
				onConnected?()

				port.on 'data', (data) ->
					if data[0..1] is 'OK'
						console.log 'OK'
					else if data[0..1] is 'KO'
						console.log 'KO'
				port.flush()

			port.on 'error', (err) -> alert err.message
	return {
		connected: -> connected
		onConnected: (callback) -> onConnected = callback
		flashRobot: flashRobot
	}
