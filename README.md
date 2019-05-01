# DSP_lib_common

This is a work in progress so things may change.

It's a development of my DSP experiments started at https://github.com/bord81/ADC-001-DSP

Now it's not tied to any hardware, works only with WAV formats and requires libsndfile which you can get from http://www.mega-nerd.com/libsndfile/ or https://github.com/erikd/libsndfile

The libary is designed to be used as .so and there are also some convenience scripts:

dsp_lookup.py - to check file sample rate and samples count

dsp_filters.py - to apply library filters to a file

dsp_graphs.py - to visualize data


Note: some WAVs don't get processed correctly and this is sometimes caused by libsndfile issues and sometimes by Matplotlib. For this project I don't wanna dive too deeply into this and you can download working samples from http://www.moviewavs.com/ or if you have BBB with ADC cape you can get working WAVs using my ADC-001-DSP project.
