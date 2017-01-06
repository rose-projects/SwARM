{BrowserWindow, app, ipcMain, dialog} = require 'electron'
serialport = require 'serialport'

window = null
renderer = null
port = null

robotsHist = []
meanDepth = 4 # TODO : add GUI control

computeMean = (robots) ->
	res = [0, 0]

	robotsHist.push robots[..] if typeof robots[0] is 'object' and robots[0].length == 2
	if robotsHist.length == meanDepth
		for rb in robotsHist when typeof rb[0] is 'object' and rb[0].length == 2
			res[0] += rb[0][0]
			res[1] += rb[0][1]
		robotsHist.shift()

	res[0] = res[0]/meanDepth
	res[1] = res[1]/meanDepth
	return [res]

app.on 'ready', ->
	# create the main window
	window = new BrowserWindow {width: 900, height: 700}
	window.loadURL "file://#{__dirname}/../ressources/index.html"
	renderer = window.webContents

ipcMain.on 'ready', ->
	serialport.list (err, ports) ->
		renderer.send('seriallist', ports.reverse()) unless err?

ipcMain.on 'connect', (event, serialpath, baudrate) ->
	port.close() if port? and port.isOpen()
	port = new serialport serialpath,
		baudRate: baudrate
		parser: serialport.parsers.readline('\n')

	port.on 'open', ->
		renderer.send 'msg', '(Port open)'
		port.on 'data', (data) ->
			if data[0..4] is 'POS :'
				coords = data[6..].split(' ')
				robots = ([parseInt(coords[i]), parseInt(coords[i+1])] for i in [0..coords.length-1] when i%2 == 0)
				renderer.send 'robots', computeMean(robots)
			else
				renderer.send 'msg', data+'<br>'
		port.flush()

	port.on 'error', (err) -> renderer.send 'msg', err.message

ipcMain.on 'beacons', (event, sb1x, sb2y) ->
	if port? and port.isOpen()
		port.write "beacon #{sb1x} #{sb2y}\r\n"

ipcMain.on 'calib', (event, id, x, y, sb1, sb2) ->
	distance = (point1, point2) -> Math.sqrt((point1[0]-point2[0])**2 + (point1[1]-point2[1])**2)
	beacons = [ ['mb', Math.round(distance([x, y], [0, 0]))],
			    ['sb1', Math.round(distance([x, y], sb1))],
			    ['sb2', Math.round(distance([x, y], sb2))] ]
	sendCal = ->
		console.log
		if robotsHist.length == 0
			setTimeout sendCal, 400
		else
			beacon = beacons.shift()
			port.write "#{beacon[0]}cal #{id} #{beacon[1]}\r\n"
			robotsHist = []
			setTimeout(sendCal, 800) if beacons.length > 0

	if port? and port.isOpen()
		sendCal()
