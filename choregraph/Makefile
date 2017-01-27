all:
	coffee -o lib/ -c src/

watch:
	coffee -o lib/ -cw src/

clean:
	rm -rf lib/

run: all
	electron .

dist: all
	rm -rf dist/
	electron-packager ./ --platform=darwin,linux --arch=x64 --out=./dist/

install:
	npm install
	./node_modules/.bin/electron-rebuild -$(electron -v)

.PHONY: all watch clean test run dist install
