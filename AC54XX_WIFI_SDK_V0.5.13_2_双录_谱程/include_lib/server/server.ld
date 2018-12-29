
    _audio_decoder_begin = .;
    PROVIDE(audio_decoder_begin = .);
        *(.audio_decoder)
    _audio_decoder_end = .;
    PROVIDE(audio_decoder_end = .);

    _audio_pkg_begin = .;
    PROVIDE(audio_pkg_begin = .);
        *(.audio_pkg)
    _audio_pkg_end = .;
    PROVIDE(audio_pkg_end = .);

    _audio_enc_fsys_begin = .;
    PROVIDE(audio_enc_fsys_begin = .);
        *(.audio_enc_fsys)
    _audio_enc_fsys_end = .;
    PROVIDE(audio_enc_fsys_end = .);


