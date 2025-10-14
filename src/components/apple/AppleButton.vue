<template>
  <button
    :class="[
      'apple-button',
      `apple-button--${variant}`,
      `apple-button--${size}`,
      {
        'apple-button--loading': loading,
        'apple-button--disabled': disabled,
        'apple-button--icon-only': iconOnly
      }
    ]"
    :disabled="disabled || loading"
    @click="handleClick"
  >
    <span v-if="loading" class="apple-button__spinner">
      <svg class="apple-spinner" viewBox="0 0 24 24">
        <circle cx="12" cy="12" r="10" fill="none" stroke="currentColor" stroke-width="2" />
      </svg>
    </span>
    <span v-if="icon && !loading" class="apple-button__icon">
      <component :is="icon" />
    </span>
    <span v-if="!iconOnly" class="apple-button__text">
      <slot />
    </span>
  </button>
</template>

<script setup>
import { defineProps, defineEmits } from 'vue'

const props = defineProps({
  variant: {
    type: String,
    default: 'primary',
    validator: (value) => ['primary', 'secondary', 'text', 'danger'].includes(value)
  },
  size: {
    type: String,
    default: 'medium',
    validator: (value) => ['small', 'medium', 'large'].includes(value)
  },
  loading: {
    type: Boolean,
    default: false
  },
  disabled: {
    type: Boolean,
    default: false
  },
  icon: {
    type: [String, Object],
    default: null
  },
  iconOnly: {
    type: Boolean,
    default: false
  }
})

const emit = defineEmits(['click'])

const handleClick = (event) => {
  if (!props.loading && !props.disabled) {
    emit('click', event)
  }
}
</script>

<style scoped>
.apple-button {
  position: relative;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: var(--space-2);
  font-family: var(--font-family-primary);
  font-weight: 500;
  line-height: var(--line-height-compact);
  text-align: center;
  border: none;
  cursor: pointer;
  transition: var(--transition-button);
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  user-select: none;
}

.apple-button:focus-visible {
  outline: 2px solid var(--apple-blue);
  outline-offset: 2px;
}

.apple-button:active:not(.apple-button--disabled) {
  transform: scale(0.98);
}

/* Sizes */
.apple-button--small {
  padding: var(--space-2) var(--space-4);
  font-size: var(--font-size-sm);
  border-radius: var(--radius-sm);
  min-height: 32px;
}

.apple-button--medium {
  padding: var(--space-3) var(--space-6);
  font-size: var(--font-size-base);
  border-radius: var(--radius-md);
  min-height: 40px;
}

.apple-button--large {
  padding: var(--space-4) var(--space-8);
  font-size: var(--font-size-lg);
  border-radius: var(--radius-md);
  min-height: 48px;
}

.apple-button--icon-only {
  padding: var(--space-2);
  aspect-ratio: 1;
}

.apple-button--icon-only.apple-button--small {
  width: 32px;
  height: 32px;
}

.apple-button--icon-only.apple-button--medium {
  width: 40px;
  height: 40px;
}

.apple-button--icon-only.apple-button--large {
  width: 48px;
  height: 48px;
}

/* Variants */
.apple-button--primary {
  background-color: var(--apple-blue);
  color: var(--apple-white);
}

.apple-button--primary:hover:not(.apple-button--disabled) {
  background-color: var(--apple-blue-hover);
}

.apple-button--primary:active:not(.apple-button--disabled) {
  background-color: var(--apple-blue-active);
}

.apple-button--secondary {
  background-color: var(--color-bg-secondary);
  color: var(--color-text-primary);
  border: 1px solid var(--color-border-default);
}

.apple-button--secondary:hover:not(.apple-button--disabled) {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.apple-button--text {
  background-color: transparent;
  color: var(--apple-blue);
  padding-left: var(--space-2);
  padding-right: var(--space-2);
}

.apple-button--text:hover:not(.apple-button--disabled) {
  background-color: var(--color-bg-secondary);
}

.apple-button--danger {
  background-color: var(--apple-red);
  color: var(--apple-white);
}

.apple-button--danger:hover:not(.apple-button--disabled) {
  background-color: #ff4545;
}

/* States */
.apple-button--disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.apple-button--loading {
  cursor: wait;
}

.apple-button--loading .apple-button__text {
  opacity: 0.6;
}

/* Icon */
.apple-button__icon {
  display: inline-flex;
  align-items: center;
  font-size: 1.2em;
}

/* Spinner */
.apple-button__spinner {
  position: absolute;
  left: 50%;
  top: 50%;
  transform: translate(-50%, -50%);
  display: inline-flex;
}

.apple-spinner {
  width: 1em;
  height: 1em;
  animation: spin 0.8s linear infinite;
}

.apple-spinner circle {
  stroke-dasharray: 60;
  stroke-dashoffset: 30;
  stroke-linecap: round;
}

@keyframes spin {
  from {
    transform: rotate(0deg);
  }
  to {
    transform: rotate(360deg);
  }
}
</style>
