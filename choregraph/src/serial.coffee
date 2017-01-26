serialport = require 'serialport'
$ = require 'jquery'

port = null
connected = no
onConnected = null
commandList = []

intRead = (cl, def) ->
	val = parseInt $(".#{cl}").val()
	return if isNaN(val) then def else val

flashRobot = (id, robot) ->
	if port? and port?.isOpen()
		console.log 'flashing robot', robot.index, 'to ID', id
		$('.flash-console').show().append("<br> => Flashing dance from robot #{robot.index+1} to ID #{id} ...<br><br>")
		$('.darkscreen').show()
			.click ->
				$('.darkscreen').hide()
				$('.flash-console').hide()

		commandList= ["clear #{id}\r\n"]

		cmd = "moves #{id}"
		cmdCnt = 0
		for mv in robot.moves
			cmd += " #{mv.date} #{mv.x} #{mv.y} #{mv.angle} #{mv.startradius} #{mv.endradius}"
			cmdCnt++
			if cmdCnt == 5
				cmdCnt = 0
				commandList.push cmd + '\r\n'
				cmd = "moves #{id}"
		commandList.push(cmd + '\r\n') unless cmdCnt == 0

		cmd = "colors #{id}"
		cmdCnt = 0
		for cl in robot.colors
			cmd += " #{cl.date} #{cl.h} #{cl.s} #{cl.v} #{cl.fade}"
			cmdCnt++
			if cmdCnt == 5
				cmdCnt = 0
				commandList.push cmd + '\r\n'
				cmd = "colors #{id}"
		commandList.push(cmd + '\r\n') unless cmdCnt == 0

		commandList.push "flash #{id}\r\n"
		$('.flash-console').append "#{commandList[0]}<br>"
		port.write commandList.shift()

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
					$('.flash-console').append "#{data}<br>" unless data[0..4] is 'POS :'

					if data[0..1] is 'OK'
						if commandList.length > 0
							$('.flash-console').append "#{commandList[0]}<br>"
							port.write commandList.shift()
				port.flush()

			port.on 'error', (err) -> alert err.message
	return {
		connected: -> connected
		onConnected: (callback) -> onConnected = callback
		flashRobot: flashRobot
	}
