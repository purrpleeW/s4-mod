module(..., package.seeall)

require("ui.widgets.PlayMenuButton")
require("ui.frontEnd.mp.IntensityButton")

local f0_local0 = 10 * _1080p
local f0_local1 = 40 * _1080p
local f0_local2 = 5 * _1080p

local PostLoadFunc = function(element, controllerIndex, controller)
	element.SetupButton = function() end
	element:SetIsContractible(false)
	element.AccordionButton.Background.BackgroundImage:setImage(RegisterMaterial("v_ui_img_buttonBg_quickPlay"), 0)
	element:addEventHandler("accordion_over", function(a1, a2)
		ACTIONS.AnimateSequence(element, "ExpandedButtonOver")
	end)
	element:addEventHandler("accordion_up", function(a1, a2)
		ACTIONS.AnimateSequence(element, "ExpandedButtonUp")
	end)
	element:addEventHandler("disable", function(a1, a2)
		element.Text:setText(ToUpperCase(lfxCall("Engine.Localize", "LUA_MENU_MP/BUTTON_DISABLED_NOT_HOST")))
	end)
	element:addEventHandler("enable", function(a1, a2)
		element.Text:setText(ToUpperCase(element:GetDataSource().name:GetValue(controllerIndex)))
	end)
	element:addEventHandler("button_action", function(a1, a2)
		--f7_arg0(element, f9_arg1.controller or f7_arg0, f7_arg0:GetDataSource())
		return true
	end)
	element:addEventHandler("lose_focus", function(a2, a2)
		element:dispatchEventToParent(a2)
	end)
	if IsLanguageTraditionalChinese() then
		ACTIONS.AnimateSequence(element, "CHT")
	elseif IsLanguageArabic() then
		ACTIONS.AnimateSequence(element, "AR")
	end
	element.Intensity:SetFocusable(false)
end

function QuickPlayButton(menu, controller)
	local self = MenuBuilder.BuildRegisteredType("PlayMenuButton", controller)

	self:SetAnchorsAndPosition(0, 1, 0, 1, 0, 400 * _1080p, 0, 128 * _1080p)
	self.id = "QuickPlayButton"
	self._animationSets = self._animationSets or {}
	self._sequences = self._sequences or {}
	self._sequenceLengths = self._sequenceLengths or {}

	local controllerIndex = LUI.ValidateControllerIndex(self, controller and controller.controllerIndex)

	self.AccordionButton.Background.BackgroundImage:SetVisibility(LUI.Visibility.Visible)
	self.AccordionButton.Background.BackgroundImage:setImage(RegisterMaterial("v_ui_img_buttonbg_quickplay"), 0)
	self.AccordionButton.Background.Darkening:SetVisibility(LUI.Visibility.Visible)
	self.Text:setText(ToUpperCase(lfxCall("Engine.Localize", "LUA_MENU/QUICK_PLAY")), 0)
	self.Description:setText(lfxCall("Engine.Localize", "LUA_MENU/QUICK_PLAY_DESC"), 0)
	self.DisabledText:setText(lfxCall("Engine.Localize", "MENU/LOCKED"), 0)

	self.Intensity = MenuBuilder.BuildRegisteredType("IntensityButton", {
		controllerIndex = controllerIndex
	})
	self.Intensity.id = "Intensity"
	self.Intensity.Backer:SetAlpha(0.75, 0)
	self.Intensity.Text:setText(lfxCall("Engine.Localize", "LUA_MENU_MP/INTENSITY_ALL"), 0)
	self.Intensity:SetAnchorsAndPosition(1, 0, 0, 1, _1080p * -250, _1080p * -150, _1080p * 3, _1080p * 31)
	self:addElement(self.Intensity)

	self.FilteredLabel = LUI.UIStyledText.new()
	self.FilteredLabel.id = "FilteredLabel"
	self.FilteredLabel:SetRGBFromTable(SWATCHES.text.buttonTextDefault, 0)
	self.FilteredLabel:SetAlpha(0.6, 0)
	self.FilteredLabel:setText("FilteredLabel", 0)
	self.FilteredLabel:setCaseStyle(LUI.CaseStyle.Uppercase)
	self.FilteredLabel:SetFont(FONTS.GetFont(FONT_STYLES.fontStyle_XSHeaderBold.font.File))
	self.FilteredLabel:SetFontSize(FONT_STYLES.fontStyle_XSHeaderBold.size)
	self.FilteredLabel:SetMinFontSize(FONT_STYLES.fontStyle_XSHeaderBold.minSize)
	self.FilteredLabel:SetWordWrap(false)
	self.FilteredLabel:SetTracking(1 * _1080p, 0)
	self.FilteredLabel:SetAlignment(LUI.Alignment.Right)
	self.FilteredLabel:SetAnchorsAndPosition(1, 0, 0, 1, _1080p * -163, _1080p * -8, _1080p * 5, _1080p * 29)
	self:addElement(self.FilteredLabel)

	self._sequenceLengths.DefaultSequence = 0
	self._sequences.DefaultSequence = function() end

	self.Intensity:RegisterAnimationSequence("all", {
		{ duration = 0, property = TWEEN_PROPERTY.RGB, value = SWATCHES.vIntensity.vIntensityAll, child = self.Intensity.Backer },
		{ duration = 0, property = TWEEN_PROPERTY.Text, value = lfxCall("Engine.Localize", "LUA_MENU_MP/INTENSITY_ALL"), child = self.Intensity.Text },
		{ value = 0.35, duration = 0, property = TWEEN_PROPERTY.Alpha, child = self.Intensity.Backer }
	})

	self._sequenceLengths.all = 0
	self._sequences.all = function()
		self.Intensity:AnimateSequence("all")
	end

	self.Intensity:RegisterAnimationSequence("low", {
		{ duration = 0, property = TWEEN_PROPERTY.RGB, value = SWATCHES.vIntensity.vIntensityTactical, child = self.Intensity.Backer },
		{ duration = 0, property = TWEEN_PROPERTY.Text, value = lfxCall("Engine.Localize", "LUA_MENU_MP/INTENSITY_PREF_LOW"), child = self.Intensity.Text },
		{ value = 1, duration = 0, property = TWEEN_PROPERTY.Alpha, child = self.Intensity.Backer }
	})

	self._sequenceLengths.low = 0
	self._sequences.low = function()
		self.Intensity:AnimateSequence("low")
	end

	self.Intensity:RegisterAnimationSequence("medium", {
		{ duration = 0, property = TWEEN_PROPERTY.RGB, value = SWATCHES.vIntensity.vIntensityStandard, child = self.Intensity.Backer },
		{ duration = 0, property = TWEEN_PROPERTY.Text, value = lfxCall("Engine.Localize", "LUA_MENU_MP/INTENSITY_PREF_MEDIUM"), child = self.Intensity.Text },
		{ value = 1, duration = 0, property = TWEEN_PROPERTY.Alpha, child = self.Intensity.Backer }
	})

	self._sequenceLengths.medium = 0
	self._sequences.medium = function()
		self.Intensity:AnimateSequence("medium")
	end

	self.Intensity:RegisterAnimationSequence("high", {
		{ duration = 0, property = TWEEN_PROPERTY.Text, value = lfxCall("Engine.Localize", "LUA_MENU_MP/INTENSITY_PREF_HIGH"), child = self.Intensity.Text },
		{ duration = 0, property = TWEEN_PROPERTY.RGB, value = SWATCHES.vIntensity.vIntensityBlitz, child = self.Intensity.Backer },
		{ value = 1, duration = 0, property = TWEEN_PROPERTY.Alpha, child = self.Intensity.Backer }
	})

	self._sequenceLengths.high = 0
	self._sequences.high = function()
		self.Intensity:AnimateSequence("high")
	end

	PostLoadFunc(self, controllerIndex, controller)
	return self
end

MenuBuilder.registerType("QuickPlayButton", QuickPlayButton)
LockTable(_M)
