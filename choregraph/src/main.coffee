{BrowserWindow, app, ipcMain, dialog} = require 'electron'
serialport = require 'serialport'
renderer = null

app.on 'ready', ->
	# create the main window
	window = new BrowserWindow {width: 1280, height: 700}
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
				console.log data
		port.flush()

	port.on 'error', (err) -> console.log err.message
