serialport = require 'serialport'
$ = require 'jquery'

port = null
connected = no
onConnected = null
commandList = []
dancing = no

intRead = (cl, def) ->
	val = parseInt $(".#{cl}").val()
	return if isNaN(val) then def else val

showSerialWindow = (textToAppend) ->
	$('.flash-console').show().append(textToAppend)
	$('.darkscreen').show()
		.click ->
			$('.darkscreen').hide()
			$('.flash-console').hide()
			$('.shell-input').hide()

toggleDancing = ->
	if port? and port?.isOpen()
		commandList = []

		dancing = not dancing
		$('.dance-btn').html(if dancing then '<span class="fa fa-stop"></span> Stop' else '<span class="fa fa-music"></span> Danser !')
			.css('font-family', 'Kirvy')
		command = if dancing then 'dance' else 'stop'
		showSerialWindow('<br>')
		port.write command + '\r\n'

		# start/stop simu at the same time
		if dancing
			$('.stop-btn').click()
			$('.play-btn').click()
		else
			$('.stop-btn').click()

showShell = ->
	if port? and port?.isOpen()
		showSerialWindow('<br>')
		$('.shell-input').show().focus()
			.keyup (e) ->
				if e.keyCode == 13
					port.write($('.shell-input').val() + '\r\n')
					$('.shell-input').val('')

flashRobot = (id, robot) ->
	if port? and port?.isOpen()
		showSerialWindow("<br> => Flashing dance from robot #{robot.index+1} to ID #{id} ...<br><br>")

		commandList= ["clear #{id}\r\n"]

		cmd = "moves #{id}"
		cmdCnt = 0
		for mv in robot.moves when mv.index != 0
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
		console.log commandList
		#$('.flash-console').append "#{commandList[0]}<br>"
		port.write commandList.shift()

module.exports = ->
	$ ->
		$('.darkscreen').hide()
		$('.flash-console').hide()

		serialport.list (err, ports) ->
			return if err?
			for p in ports.reverse()
				$('.serialpath').append "<option value=\"#{p.comName}\">#{p.comName}</option>"

		$('.dance-btn').click toggleDancing
		$('.shell-btn').click showShell
		$('.serial-btn').click ->
			connected = no
			port.close() if port? and port?.isOpen()
			port = new serialport $('.serialpath').val(),
				baudRate: intRead('baudrate', 38400)
				parser: serialport.parsers.readline('\n')

			port.on 'open', ->
				connected = yes
				onConnected?()
				$('.shell-btn').removeClass 'disabled'
				$('.dance-btn').removeClass 'disabled'

				port.on 'data', (data) ->
					unless data[0..4] is 'POS :'
						$('.flash-console').append("#{data}<br>").scrollTop($('.flash-console')[0].scrollHeight)

					if data[0..1] is 'OK'
						if commandList.length > 0
							#$('.flash-console').append "#{commandList[0]}<br>"
							port.write commandList.shift()
				port.flush()

			port.on 'error', (err) -> alert err.message
	return {
		connected: -> connected
		onConnected: (callback) -> onConnected = callback
		flashRobot: flashRobot
	}
