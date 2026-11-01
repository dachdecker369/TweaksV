// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the RayTracing part: the game's ray tracing buffer of moving geometry - the one vertex buffer its GPU
// animation skins every moving object into each frame, before the ray tracing refits them - raised to
// DYNAMIC_BVH_VERTICES, so that a scene of many high-poly vehicles and peds keeps their ray traced shadows, reflections
// and occlusion (SPECIFICATIONS 11.13).
#pragma once

namespace TweaksV
{
	// The RayTracing part, started by its entry of the feature table, from DllMain, while [RAYTRACING] is on: the buffer's
	// size and its two checks raised together, all or nothing, before the game makes the buffer; with DIAGNOSTIC_MODE=1 its
	// sites logged and its measurement started, also with DYNAMIC_BVH_VERTICES=0.
	void InitRayTracing();
}
