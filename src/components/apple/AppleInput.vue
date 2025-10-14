<template>
  <div
    :class="[
      'apple-input-wrapper',
      {
        'apple-input-wrapper--focused': isFocused,
        'apple-input-wrapper--disabled': disabled,
        'apple-input-wrapper--error': error
      }
    ]"
  >
    <label v-if="label" :for="inputId" class="apple-input__label">
      {{ label }}
      <span v-if="required" class="apple-input__required">*</span>
    </label>

    <div class="apple-input__container">
      <span v-if="prefixIcon" class="apple-input__prefix">
        <component :is="prefixIcon" />
      </span>

      <input
        :id="inputId"
        v-model="internalValue"
        :type="type"
        :placeholder="placeholder"
        :disabled="disabled"
        :readonly="readonly"
        :maxlength="maxlength"
        class="apple-input"
        @focus="handleFocus"
        @blur="handleBlur"
        @input="handleInput"
        @change="handleChange"
      />

      <span v-if="suffixIcon || showClearButton" class="apple-input__suffix">
        <button
          v-if="showClearButton && internalValue"
          type="button"
          class="apple-input__clear"
          @click="handleClear"
        >
          ✕
        </button>
        <component v-else-if="suffixIcon" :is="suffixIcon" />
      </span>
    </div>

    <div v-if="error || helperText" class="apple-input__helper">
      <span v-if="error" class="apple-input__error">{{ error }}</span>
      <span v-else-if="helperText" class="apple-input__helper-text">{{ helperText }}</span>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'

const props = defineProps({
  modelValue: {
    type: [String, Number],
    default: ''
  },
  type: {
    type: String,
    default: 'text'
  },
  label: {
    type: String,
    default: ''
  },
  placeholder: {
    type: String,
    default: ''
  },
  helperText: {
    type: String,
    default: ''
  },
  error: {
    type: String,
    default: ''
  },
  disabled: {
    type: Boolean,
    default: false
  },
  readonly: {
    type: Boolean,
    default: false
  },
  required: {
    type: Boolean,
    default: false
  },
  clearable: {
    type: Boolean,
    default: false
  },
  maxlength: {
    type: [String, Number],
    default: undefined
  },
  prefixIcon: {
    type: [String, Object],
    default: null
  },
  suffixIcon: {
    type: [String, Object],
    default: null
  }
})

const emit = defineEmits(['update:modelValue', 'focus', 'blur', 'input', 'change', 'clear'])

const internalValue = ref(props.modelValue)
const isFocused = ref(false)
const inputId = computed(() => `apple-input-${Math.random().toString(36).substr(2, 9)}`)
const showClearButton = computed(() => props.clearable && !props.disabled && !props.readonly)

watch(() => props.modelValue, (newValue) => {
  internalValue.value = newValue
})

watch(internalValue, (newValue) => {
  emit('update:modelValue', newValue)
})

const handleFocus = (event) => {
  isFocused.value = true
  emit('focus', event)
}

const handleBlur = (event) => {
  isFocused.value = false
  emit('blur', event)
}

const handleInput = (event) => {
  emit('input', event.target.value)
}

const handleChange = (event) => {
  emit('change', event.target.value)
}

const handleClear = () => {
  internalValue.value = ''
  emit('clear')
}
</script>

<style scoped>
.apple-input-wrapper {
  display: flex;
  flex-direction: column;
  gap: var(--space-2);
}

.apple-input__label {
  font-size: var(--font-size-sm);
  font-weight: 500;
  color: var(--color-text-primary);
  line-height: var(--line-height-compact);
}

.apple-input__required {
  color: var(--apple-red);
  margin-left: var(--space-1);
}

.apple-input__container {
  position: relative;
  display: flex;
  align-items: center;
}

.apple-input {
  width: 100%;
  padding: var(--space-3) var(--space-4);
  font-family: var(--font-family-primary);
  font-size: var(--font-size-base);
  line-height: var(--line-height-normal);
  color: var(--color-text-primary);
  background-color: var(--color-bg-secondary);
  border: 1px solid var(--color-border-default);
  border-radius: var(--radius-sm);
  transition: var(--transition-input);
  outline: none;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

.apple-input:hover:not(:disabled):not(:readonly) {
  background-color: var(--color-bg-tertiary);
  border-color: var(--color-border-hover);
}

.apple-input:focus {
  background-color: var(--color-bg-primary);
  border-color: var(--apple-blue);
  box-shadow: 0 0 0 3px var(--apple-blue-alpha);
}

.apple-input:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.apple-input:readonly {
  background-color: var(--color-bg-secondary);
  cursor: default;
}

.apple-input::placeholder {
  color: var(--color-text-tertiary);
}

/* Prefix/Suffix Icons */
.apple-input__prefix,
.apple-input__suffix {
  position: absolute;
  display: flex;
  align-items: center;
  color: var(--color-text-secondary);
  font-size: 1.2em;
  pointer-events: none;
}

.apple-input__prefix {
  left: var(--space-4);
}

.apple-input__suffix {
  right: var(--space-4);
}

.apple-input__container:has(.apple-input__prefix) .apple-input {
  padding-left: calc(var(--space-4) * 2 + 1.2em);
}

.apple-input__container:has(.apple-input__suffix) .apple-input {
  padding-right: calc(var(--space-4) * 2 + 1.2em);
}

/* Clear Button */
.apple-input__clear {
  pointer-events: all;
  padding: var(--space-1);
  font-size: var(--font-size-sm);
  color: var(--color-text-tertiary);
  background-color: transparent;
  border: none;
  border-radius: 50%;
  cursor: pointer;
  transition: var(--transition-fast);
  display: flex;
  align-items: center;
  justify-content: center;
}

.apple-input__clear:hover {
  color: var(--color-text-primary);
  background-color: var(--color-bg-tertiary);
}

/* Helper Text */
.apple-input__helper {
  font-size: var(--font-size-xs);
  line-height: var(--line-height-compact);
}

.apple-input__helper-text {
  color: var(--color-text-secondary);
}

.apple-input__error {
  color: var(--apple-red);
}

/* Error State */
.apple-input-wrapper--error .apple-input {
  border-color: var(--apple-red);
}

.apple-input-wrapper--error .apple-input:focus {
  border-color: var(--apple-red);
  box-shadow: 0 0 0 3px rgba(255, 59, 48, 0.1);
}

/* Responsive */
@media (max-width: 768px) {
  .apple-input {
    font-size: var(--font-size-base);
    padding: var(--space-3);
  }
}
</style>
