// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the lighting capacity, a limit of a part to come, measured first. A frame's lights pass through layers of fixed
// room: a job's list of 80 lights, the frame's stores of 400 lights and 928 coronas, 300 vehicles a frame, the scene store's
// 1024, the ray traced list's 1024 and its upload's 512, the reflection lists' 768 and the corona store's 1088 - and many
// vehicles with real siren lights are where a load may reach them. With DIAGNOSTIC_MODE=1 the module samples how far each
// layer fills and counts the lights a job's full list refuses, and logs them (SPECIFICATIONS 11.16.3).
#pragma once

namespace TweaksV
{
	// Finds and checks the lighting capacity's sites, starts its measurement and redirects the one jump its count of refused
	// lights rests on. Called by the table of the parts measured first, with DIAGNOSTIC_MODE=1 only.
	void MeasureLightCapacity();
}
