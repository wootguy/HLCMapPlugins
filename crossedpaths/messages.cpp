/**
 * Make it easier to create message parameters.
 */

const string INFO_MSG = "info";
const string SUCCESS_MSG = "success";
const string WARNING_MSG = "warning";


HUDTextParams CreateHudTextParams(const string &in tone, const int &in channel) {
    auto hudTextParams = HUDTextParams();

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

    if (INFO_MSG == tone) {
        hudTextParams.r1 = 200;
        hudTextParams.g1 = 200;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 255;
        hudTextParams.g2 = 255;
        hudTextParams.b2 = 255;
    } else if (SUCCESS_MSG == tone) {
        hudTextParams.r1 = 200;
        hudTextParams.g1 = 255;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 50;
        hudTextParams.g2 = 255;
        hudTextParams.b2 = 50;
    } else if (WARNING_MSG == tone) {
        hudTextParams.r1 = 255;
        hudTextParams.g1 = 200;
        hudTextParams.b1 = 200;

        hudTextParams.r2 = 255;
        hudTextParams.g2 = 50;
        hudTextParams.b2 = 50;
    }

    return hudTextParams;
}