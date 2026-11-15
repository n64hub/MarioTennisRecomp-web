// RecompFrontend includes this file from the game's patches/ (recompui/src/api/ui_api_events.cpp), for the UI events
// it hands the game's patches. Mario Tennis has no patches, so these three headers are all that folder holds.
#ifndef __UI_FUNCS_INTERNAL_H__
#define __UI_FUNCS_INTERNAL_H__

#include "patch_helpers.h"
#include "recompui_event_structs.h"

DECLARE_FUNC(void, recomp_run_ui_callbacks);

#endif
