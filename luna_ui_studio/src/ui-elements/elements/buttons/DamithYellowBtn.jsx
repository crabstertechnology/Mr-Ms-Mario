// ============================================================
// UIverse Button by Damithkumara (green-chipmunk-67)
// Source: https://uiverse.io/Damithkumara/green-chipmunk-67
// This is the EXACT React component from UIverse — no conversion needed!
// ============================================================
import React from 'react'
import styled from 'styled-components'

const DamithYellowBtn = ({ label = 'BUTTON', borderColor = '#ffff00', bgColor = '#363636', textColor = '#ffff00' }) => {
  return (
    <StyledWrapper $border={borderColor} $bg={bgColor} $text={textColor} $label={label}>
      <button type="button" data-label={label}><span />{label}</button>
    </StyledWrapper>
  )
}

const StyledWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;
  background: ${p => p.$bg};
  overflow: hidden;

  button {
    padding: 0.9em 1.8em;
    text-transform: uppercase;
    text-decoration: none;
    letter-spacing: 4px;
    color: transparent;
    border: 3px solid ${p => p.$border};
    font-size: 14px;
    position: relative;
    font-family: inherit;
    background: transparent;
    cursor: pointer;
    white-space: nowrap;
  }

  button::before {
    content: "${p => p.$label || 'BUTTON'}";
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-color: ${p => p.$bg};
    color: ${p => p.$text};
    display: flex;
    justify-content: center;
    align-items: center;
    transition: all 0.5s;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 4px;
  }

  button:hover::before {
    left: 100%;
    transform: scale(0) rotateY(360deg);
    opacity: 0;
  }

  button::after {
    content: "${p => p.$label || 'BUTTON'}";
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background-color: ${p => p.$bg};
    color: ${p => p.$text};
    display: flex;
    justify-content: center;
    align-items: center;
    transition: all 0.5s;
    transform: scale(0) rotateY(0deg);
    opacity: 0;
    font-size: 14px;
    text-transform: uppercase;
    letter-spacing: 4px;
  }

  button:hover::after {
    left: 0;
    transform: scale(1) rotateY(360deg);
    opacity: 1;
  }
`

export default DamithYellowBtn
