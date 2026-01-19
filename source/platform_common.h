
#pragma once

namespace tyon
{
    struct platform_procs
    {
        typed_procedure<fresult()> init;
        typed_procedure<fresult()> tick;
        typed_procedure<fresult()> destroy;
        typed_procedure<fresult()> window_open;
        typed_procedure<fresult()> window_close;
    };
}
