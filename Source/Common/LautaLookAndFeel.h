#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace LautaUI
{

class LautaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    LautaLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff121318));
        setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1e2029));
        setColour (juce::ComboBox::textColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff333846));
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1a1c24));
        setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff2dd4bf));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff0f172a));
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff222530));
        setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2dd4bf));
        setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff0f172a));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto center = bounds.getCentre();

        // Accent color (default cyan/teal, custom color per macro)
        juce::Colour accentColour = slider.findColour (juce::Slider::rotarySliderFillColourId);
        if (accentColour.isTransparent())
            accentColour = juce::Colour (0xff2dd4bf);

        const bool isMacro = slider.getProperties().contains ("isMacro");

        // Background Track Arc
        const float trackWidth = isMacro ? 6.5f : 4.0f;
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (center.x, center.y, radius - trackWidth, radius - trackWidth,
                                     0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff1f222c));
        g.strokePath (backgroundArc, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Active Value Arc
        if (sliderPosProportional > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (center.x, center.y, radius - trackWidth, radius - trackWidth,
                                    0.0f, rotaryStartAngle, toAngle, true);

            g.setColour (accentColour);
            g.strokePath (valueArc, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Inner Dial Body
        const float knobRadius = radius - trackWidth - (isMacro ? 7.0f : 4.0f);
        juce::Rectangle<float> knobBounds (center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        juce::ColourGradient knobGrad (juce::Colour (0xff282b36), center.x, center.y - knobRadius,
                                       juce::Colour (0xff181a22), center.x, center.y + knobRadius, false);
        g.setGradientFill (knobGrad);
        g.fillEllipse (knobBounds);

        // Subtle outer border of inner dial
        g.setColour (juce::Colour (0xff383d4c));
        g.drawEllipse (knobBounds, 1.2f);

        // Indicator Needle / Pip
        juce::Path needle;
        const float needleLength = knobRadius * 0.65f;
        const float needleThickness = isMacro ? 3.0f : 2.0f;
        needle.addRoundedRectangle (-needleThickness * 0.5f, -knobRadius + 2.0f, needleThickness, needleLength, 1.0f);
        needle.applyTransform (juce::AffineTransform::rotation (toAngle).translated (center.x, center.y));

        g.setColour (accentColour.brighter (0.2f));
        g.fillPath (needle);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        const bool isOn = button.getToggleState();

        juce::Colour fillColour = isOn ? juce::Colour (0xff2dd4bf) : juce::Colour (0xff1e212b);
        if (shouldDrawButtonAsHighlighted && ! isOn)
            fillColour = fillColour.brighter (0.1f);
        if (shouldDrawButtonAsDown)
            fillColour = fillColour.darker (0.1f);

        g.setColour (fillColour);
        g.fillRoundedRectangle (bounds, 4.0f);

        g.setColour (isOn ? juce::Colour (0xff5eead4) : juce::Colour (0xff333846));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }
};

} // namespace LautaUI
