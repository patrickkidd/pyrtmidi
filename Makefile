run-tests-linux:
	python3 setup.py build
	sudo python3 setup.py install
	python3 tests/test_rtmidi.py
