// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

// utils
import { getLocale } from '../../../../common/locale'

// types
import { BraveWallet, ChartTimelineObjectType } from '../../../constants/types'
import RowReveal from '../../shared/animated-reveals/row-reveal'

// Styled Components
import { ToggleVisibilityButton } from '../../shared/style'
import {
  StyledWrapper,
  ButtonText,
  StyledButton
} from './chart-control-bar.style'

export interface Props {
  disabled?: boolean
  onDisabledChanged?: (isDisabled: boolean) => void
  onSelectTimeframe: (id: BraveWallet.AssetPriceTimeframe) => void
  selectedTimeline: BraveWallet.AssetPriceTimeframe
  timelineOptions: ChartTimelineObjectType[]
}

export const ChartControlBar = React.memo(({
  disabled,
  onDisabledChanged,
  onSelectTimeframe,
  selectedTimeline,
  timelineOptions
}: Props) => {
  return (
    <StyledWrapper>

      <RowReveal hideContent={disabled}>
        {timelineOptions.map((t) =>
          <StyledButton
            key={t.id}
            onClick={() => onSelectTimeframe(t.id)}
            isSelected={selectedTimeline === t.id}
            disabled={disabled}
          >
            <ButtonText
              isSelected={selectedTimeline === t.id}
              disabled={disabled}
            >
              {getLocale(t.name)}
            </ButtonText>
          </StyledButton>
        )}
      </RowReveal>

      {onDisabledChanged &&
        <StyledButton as="div">
          <ToggleVisibilityButton
            isVisible={!disabled}
            onClick={() => onDisabledChanged(!disabled)}
          />
        </StyledButton>
      }
    </StyledWrapper>
  )
})
