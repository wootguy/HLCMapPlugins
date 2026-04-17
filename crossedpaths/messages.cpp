#include "extdll.h"
#include "util.h"
#include "crossedpaths.h"

/**
 * Make it easier to create message parameters.
 */

hudtextparms_t CreateHudTextParams(const char* tone, int channel) {
    hudtextparms_t hudTextParams;
    memset(&hudTextParams, 0, sizeof(hudTextParams));

    hudTextParams.x = -1;
    hudTextParams.effect = 0;

    if (4 == channel) {
        hudTextParams.fadeinTime = 0;
        hudTextParams.fadeoutTime = 0.1;
        hudTextParams.holdTime = 1.8;
        hudTextParams.channel = 1;
        hudTextParams.y = 0.6;
    } else {
        hudTextParams.fadeinTime = 0.2;
        hudTextParams.fadeoutTime = 0.2;
        hudTextParams.holdTime = 3.5;
        hudTextParams.channel = channel;
        hudTextParams.y = 0.4;
    }

    if (!strcmp(INFO_MSG, tone)) {
        hudTextParams.r1 = 200;
        hudTextParams.g1 = 200;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 255;
        hudTextParams.g2 = 255;
        hudTextParams.b2 = 255;
    } else if (!strcmp(SUCCESS_MSG, tone)) {
        hudTextParams.r1 = 200;
        hudTextParams.g1 = 255;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 50;
        hudTextParams.g2 = 255;
        hudTextParams.b2 = 50;
    } else if (!strcmp(WARNING_MSG, tone)) {
        hudTextParams.r1 = 255;
        hudTextParams.g1 = 200;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 255;
        hudTextParams.g2 = 50;
        hudTextParams.b2 = 50;
    }

    return hudTextParams;
}