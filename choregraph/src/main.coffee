{BrowserWindow, app, ipcMain, dialog} = require 'electron'

app.on 'ready', ->
	# create the main window
	window = new BrowserWindow {width: 1280, height: 700}
	window.loadURL "file://#{__dirname}/../ressources/index.html"
	renderer = window.webContents
	window.on 'close', (e) ->
		renderer.send 'closing'
		e.preventDefault()
