/**
 * HiveControl Loading Spinner Utility
 * Provides functions to show/hide loading indicators during form submissions
 */

(function() {
    'use strict';

    // Create spinner overlay on page load
    var spinnerOverlay = null;

    function createSpinnerOverlay() {
        if (!spinnerOverlay) {
            spinnerOverlay = document.createElement('div');
            spinnerOverlay.className = 'hive-spinner-overlay';
            spinnerOverlay.innerHTML = '<div class="hive-spinner"></div>';
            document.body.appendChild(spinnerOverlay);
        }
        return spinnerOverlay;
    }

    // Show loading spinner
    window.HiveSpinner = {
        show: function() {
            var overlay = createSpinnerOverlay();
            overlay.classList.add('active');
        },

        hide: function() {
            if (spinnerOverlay) {
                spinnerOverlay.classList.remove('active');
            }
        },

        // Auto-attach to forms with class 'hive-form'
        attachToForms: function() {
            var forms = document.querySelectorAll('form.hive-form');

            forms.forEach(function(form) {
                // Handle form submission
                form.addEventListener('submit', function() {
                    HiveSpinner.show();
                });

                // Handle input/select change events that trigger form submission
                var inputs = form.querySelectorAll('input, select');
                inputs.forEach(function(input) {
                    var onchangeAttr = input.getAttribute('onchange');
                    if (onchangeAttr && onchangeAttr.includes('submit')) {
                        input.addEventListener('change', function() {
                            HiveSpinner.show();
                        });
                    }
                });
            });
        },

        // Show inline spinner next to an element
        showInline: function(elementId) {
            var element = document.getElementById(elementId);
            if (element) {
                var spinner = document.createElement('span');
                spinner.className = 'hive-spinner-inline';
                spinner.id = elementId + '-spinner';
                element.parentNode.insertBefore(spinner, element.nextSibling);
            }
        },

        // Hide inline spinner
        hideInline: function(elementId) {
            var spinner = document.getElementById(elementId + '-spinner');
            if (spinner) {
                spinner.remove();
            }
        }
    };

    // Auto-initialize on page load
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function() {
            HiveSpinner.attachToForms();
        });
    } else {
        HiveSpinner.attachToForms();
    }
})();
