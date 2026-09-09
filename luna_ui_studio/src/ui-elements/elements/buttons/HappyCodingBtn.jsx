// ============================================================
// UIverse "Happy Coding!" Neumorphic Button
// Source: UIverse.io
// Exact React Component with styled-components
// ============================================================
import React from 'react'
import styled from 'styled-components'

const HappyCodingBtn = ({
  label = 'Happy Coding!',
  bgColor = '#EEF2FF',
  borderColor = '#536DFE',
  textColor = '#536DFE',
}) => {
  return (
    <StyledWrapper $bg={bgColor} $border={borderColor} $text={textColor}>
      <button className="button" role="button">{label}</button>
    </StyledWrapper>
  )
}

const StyledWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;
  overflow: visible;

  .button {
    align-items: center;
    appearance: none;
    background-color: ${p => p.$bg || '#EEF2FF'};
    border-radius: 8px;
    border-width: 2px;
    border-style: solid;
    border-color: ${p => p.$border || '#536DFE'};
    box-shadow: rgba(83, 109, 254, 0.2) 0 2px 4px, rgba(83, 109, 254, 0.15) 0 7px 13px -3px, #D6D6E7 0 -3px 0 inset;
    box-sizing: border-box;
    color: ${p => p.$text || '#536DFE'};
    cursor: pointer;
    display: inline-flex;
    font-family: "JetBrains Mono", monospace;
    width: 100%;
    height: 100%;
    justify-content: center;
    line-height: 1;
    list-style: none;
    overflow: hidden;
    padding-left: 16px;
    padding-right: 16px;
    position: relative;
    text-align: center;
    text-decoration: none;
    transition: box-shadow 0.15s, transform 0.15s;
    user-select: none;
    -webkit-user-select: none;
    touch-action: manipulation;
    white-space: nowrap;
    will-change: box-shadow, transform;
    font-size: 14px;
    font-weight: 600;
  }

  .button:focus {
    outline: none;
    box-shadow: #D6D6E7 0 0 0 1.5px inset, rgba(83, 109, 254, 0.4) 0 2px 4px, rgba(83, 109, 254, 0.3) 0 7px 13px -3px, #D6D6E7 0 -3px 0 inset;
  }

  .button:hover {
    box-shadow: rgba(83, 109, 254, 0.3) 0 4px 8px, rgba(83, 109, 254, 0.2) 0 7px 13px -3px, #D6D6E7 0 -3px 0 inset;
    transform: translateY(-2px);
  }

  .button:active {
    box-shadow: #D6D6E7 0 3px 7px inset;
    transform: translateY(2px);
  }
`

export default HappyCodingBtn
