

namespace tyon
{
    ui_context* g_ui = nullptr;
    PROC ui_init() -> fresult
    {
        if (g_ui != nullptr)
        {   TYON_ERROR( "ui_context is already initialized" );
            return false;
        }
        g_ui = memory_allocate<ui_context>(1);
        // TODO: Load font as asset
        ui_font& font = g_ui->default_font;
        file* noto_sans_file = entity_allocate<file>();
        *noto_sans_file = file_load_binary( "data/fonts/noto_sans/NotoSans-Regular.ttf" );
        SDL_IOStream* noto_sans_io = SDL_IOFromMem(
            noto_sans_file->memory.data, noto_sans_file->memory.size );
        sdl::TTF_Font* noto_sans = sdl::TTF_OpenFontIO( noto_sans_io, true, 16 );
        sdl::TTF_SetFontHinting( noto_sans, sdl::TTF_HINTING_LIGHT_SUBPIXEL );

        font.platform_font = noto_sans;
        font.size_points = 16.0;

        return true;
    }

    PROC ui_tick() -> void
    {}

    PROC ui_destroy() -> void
    {}

    PROC ui_frame_start() -> void
    {}

    PROC ui_frame_end() -> void
    {}

}
