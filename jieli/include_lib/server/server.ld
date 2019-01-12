
    _audio_decoder_begin = .;
    PROVIDE(audio_decoder_begin = .);
        *(.audio_decoder)
    _audio_decoder_end = .;
    PROVIDE(audio_decoder_end = .);
