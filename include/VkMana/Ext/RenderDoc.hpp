/**
 * TODO:
 * - Linux/Android support - https://renderdoc.org/docs/in_application_api.html
 */

#pragma once

namespace VkMana::Ext
{
	bool RenderDocInit();
	bool RenderDocDeinit();

	void RenderDocStartFrameCapture();
	void RenderDocEndFrameCapture();

	void RenderDocLaunchReplayUI();

} // namespace VkMana
