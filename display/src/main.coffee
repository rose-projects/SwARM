{BrowserWindow, app, ipcMain, dialog} = require 'electron'
serialport = require 'serialport'

window = null
renderer = null
port = null

robotsHist = []
meanDepth = 1 # TODO : add GUI control

computeMean = (robots) ->
	res = [0, 0]

	robotsHist.push robots[..] if typeof robots[0] is 'object' and robots[0].length == 2
	if robotsHist.length == meanDepth
		for rb in robotsHist when typeof rb[0] is 'object' and rb[0].length == 2
			console.log rb[0]
			res[0] += rb[0][0]
			res[1] += rb[0][1]
		robotsHist.shift()

	res[0] = res[0]/meanDepth
	res[1] = res[1]/meanDepth
	console.log res
	return [res]

app.on 'ready', ->
	# create the main window
	window = new BrowserWindow {width: 900, height: 700}
	window.loadURL "file://#{__dirname}/../ressources/index.html"
	renderer = window.webContents

ipcMain.on 'ready', ->
	serialport.list (err, ports) ->
		console.log ports
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
