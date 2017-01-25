createPlayer = require('web-audio-player')

audio = createPlayer('../ressources/audio.mp3')
audio.on 'load', ->
	audio.play()
	audio.node.connect(audio.context.destination)

# TODO : everything
