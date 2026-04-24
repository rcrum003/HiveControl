/**
 * HiveDiagram — renders a stacked beehive SVG from configuration data.
 * Used by both the config editor (interactive) and dashboard widget (read-only).
 */
var HiveDiagram = (function () {
    'use strict';

    var COMPONENT_DEFS = {
        tele_cover: {
            label: 'Telescoping Cover',
            height: 22,
            color: '#8B8B8B',
            strokeColor: '#6d6d6d',
            overhang: 8,
            dbWeightKey: 'HIVE_TOP_TELE_COVER_WEIGHT',
            dbNumKey: 'NUM_HIVE_TOP_TELE_COVER',
            image: '/images/hive/telecover.png'
        },
        migratory_cover: {
            label: 'Migratory Cover',
            height: 18,
            color: '#D4A574',
            strokeColor: '#b8895c',
            overhang: 2,
            dbWeightKey: 'HIVE_TOP_MIGRATORY_COVER_WEIGHT',
            dbNumKey: 'NUM_HIVE_TOP_MIGRATORY_COVER',
            image: '/images/hive/migratorytop.png'
        },
        inner_cover: {
            label: 'Inner Cover',
            height: 14,
            color: '#C4956A',
            strokeColor: '#a87b52',
            overhang: 0,
            dbWeightKey: 'HIVE_TOP_INNER_COVER_WEIGHT',
            dbNumKey: 'NUM_HIVE_TOP_INNER_COVER',
            image: '/images/hive/innercover.png'
        },
        feeder: {
            label: 'Hive Feeder',
            height: 32,
            color: '#4A90D9',
            strokeColor: '#3570b0',
            overhang: 0,
            dbWeightKey: 'HIVE_FEEDER_WEIGHT',
            dbNumKey: 'NUM_HIVE_FEEDER',
            image: '/images/hive/feeder.png'
        },
        shallow: {
            label: 'Shallow Super',
            height: 42,
            color: '#DEB887',
            strokeColor: '#c49b6a',
            overhang: 0,
            dbWeightKey: 'HIVE_BODY_SHAL_FOUNDATION_WEIGHT',
            dbNumKey: 'NUM_HIVE_BODY_SHAL_FOUNDATION',
            image: '/images/hive/shallow.png'
        },
        medium: {
            label: 'Medium Super',
            height: 56,
            color: '#D2A679',
            strokeColor: '#b88c5f',
            overhang: 0,
            dbWeightKey: 'HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT',
            dbNumKey: 'NUM_HIVE_BODY_MEDIUM_FOUNDATION',
            image: '/images/hive/medium-w-frames.png'
        },
        deep: {
            label: 'Deep Body',
            height: 76,
            color: '#C89B6B',
            strokeColor: '#a88050',
            overhang: 0,
            dbWeightKey: 'HIVE_BODY_DEEP_FOUNDATION_WEIGHT',
            dbNumKey: 'NUM_HIVE_BODY_DEEP_FOUNDATION',
            image: '/images/hive/deep.png'
        },
        solid_bottom: {
            label: 'Solid Bottom Board',
            height: 18,
            color: '#B8956A',
            strokeColor: '#9a7b54',
            overhang: 12,
            dbWeightKey: 'HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT',
            dbNumKey: 'NUM_HIVE_BASE_SOLID_BOTTOM_BOARD',
            image: '/images/hive/solidbottomboard.png'
        },
        screened_bottom: {
            label: 'Screened Bottom Board',
            height: 20,
            color: '#B8956A',
            strokeColor: '#9a7b54',
            overhang: 12,
            hasScreen: true,
            dbWeightKey: 'HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT',
            dbNumKey: 'NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD',
            image: '/images/hive/screenedbottomboard2.png'
        }
    };

    var BODY_WIDTH = 160;
    var PADDING_X = 30;
    var PADDING_TOP = 20;
    var PADDING_BOTTOM = 10;
    var COMPONENT_GAP = 1;

    function escapeHtml(str) {
        var div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    function buildStackFromConfig(config) {
        if (config.stackOrder && config.stackOrder.length > 0) {
            return config.stackOrder.slice();
        }
        var stack = [];
        var order = [
            'solid_bottom', 'screened_bottom',
            'deep', 'medium', 'shallow', 'feeder',
            'inner_cover', 'migratory_cover', 'tele_cover'
        ];
        for (var i = 0; i < order.length; i++) {
            var key = order[i];
            var def = COMPONENT_DEFS[key];
            var num = parseInt(config.quantities[def.dbNumKey] || 0, 10);
            for (var j = 0; j < num; j++) {
                stack.push(key);
            }
        }
        return stack;
    }

    function computeTareWeight(stack, weights, fixedWeights) {
        var total = 0;
        for (var i = 0; i < stack.length; i++) {
            var def = COMPONENT_DEFS[stack[i]];
            total += parseFloat(weights[def.dbWeightKey] || 0);
        }
        total += parseFloat(fixedWeights.HIVE_TOP_WEIGHT || 0);
        total += parseFloat(fixedWeights.HIVE_COMPUTER_WEIGHT || 0);
        total += parseFloat(fixedWeights.HIVE_MISC_WEIGHT || 0);
        return Math.round(total * 100) / 100;
    }

    function renderSVG(container, stack, opts) {
        opts = opts || {};
        var mode = opts.mode || 'dashboard';
        var sensorPos = (opts.sensorPosition != null && opts.sensorPosition >= 0) ? opts.sensorPosition : -1;
        var sensorValue = opts.sensorValue || '';
        var sensorLabel = opts.sensorLabel || '';
        var frameFeederPos = (opts.frameFeederPosition != null && opts.frameFeederPosition >= 0) ? opts.frameFeederPosition : -1;
        var weights = opts.weights || {};
        var showLabels = (mode === 'editor') || opts.showLabels;
        var scale = opts.scale || 1;

        var totalHeight = PADDING_TOP + PADDING_BOTTOM;
        for (var i = 0; i < stack.length; i++) {
            var def = COMPONENT_DEFS[stack[i]];
            totalHeight += (def.height * scale) + COMPONENT_GAP;
        }
        if (sensorPos >= 0) totalHeight += 20;
        totalHeight = Math.max(totalHeight, 80);

        var svgWidth = BODY_WIDTH + (PADDING_X * 2) + (showLabels ? 140 : 0);
        var svgHeight = totalHeight;

        var parts = [];
        parts.push('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ' + svgWidth + ' ' + svgHeight + '" width="' + svgWidth + '" height="' + svgHeight + '" role="img" aria-label="Hive body diagram">');

        var y = PADDING_TOP;
        var cx = PADDING_X;

        // Render from top to bottom (stack array is bottom-to-top, so reverse)
        var reversed = stack.slice().reverse();
        var sensorDrawnAt = -1;
        if (sensorPos >= 0) {
            sensorDrawnAt = stack.length - 2 - sensorPos;
        }

        for (var ri = 0; ri < reversed.length; ri++) {
            var compKey = reversed[ri];
            var cdef = COMPONENT_DEFS[compKey];
            var h = cdef.height * scale;
            var oh = cdef.overhang || 0;
            var x = cx - oh;
            var w = BODY_WIDTH + (oh * 2);

            // Component rectangle
            parts.push('<rect x="' + x + '" y="' + y + '" width="' + w + '" height="' + h + '" rx="2" ry="2" fill="' + cdef.color + '" stroke="' + cdef.strokeColor + '" stroke-width="1.5" data-component="' + compKey + '" data-index="' + (stack.length - 1 - ri) + '"/>');

            // Frame lines for body components
            if (compKey === 'deep' || compKey === 'medium' || compKey === 'shallow') {
                var frameCount = compKey === 'deep' ? 9 : (compKey === 'medium' ? 8 : 7);
                var frameSpacing = w / (frameCount + 1);
                for (var fi = 1; fi <= frameCount; fi++) {
                    var fx = x + (fi * frameSpacing);
                    parts.push('<line x1="' + fx + '" y1="' + (y + 3) + '" x2="' + fx + '" y2="' + (y + h - 3) + '" stroke="' + cdef.strokeColor + '" stroke-width="0.5" opacity="0.5"/>');
                }
                // Hand holds
                var holdY = y + (h / 2) - 4;
                parts.push('<rect x="' + (x + 2) + '" y="' + holdY + '" width="14" height="8" rx="3" fill="' + cdef.strokeColor + '" opacity="0.4"/>');
                parts.push('<rect x="' + (x + w - 16) + '" y="' + holdY + '" width="14" height="8" rx="3" fill="' + cdef.strokeColor + '" opacity="0.4"/>');
            }

            // Frame feeder indicator inside body component
            if (frameFeederPos >= 0 && (compKey === 'deep' || compKey === 'medium' || compKey === 'shallow') && (stack.length - 1 - ri) === frameFeederPos) {
                var ffCenterX = x + (w / 2);
                var ffW = Math.min(24, w * 0.15);
                parts.push('<rect x="' + (ffCenterX - ffW / 2) + '" y="' + (y + 2) + '" width="' + ffW + '" height="' + (h - 4) + '" rx="1" fill="#222" stroke="#000" stroke-width="1.5"/>');
            }

            // Screen pattern for screened bottom board
            if (cdef.hasScreen) {
                var meshSpacing = 6;
                for (var mx = x + 8; mx < x + w - 8; mx += meshSpacing) {
                    parts.push('<line x1="' + mx + '" y1="' + (y + 4) + '" x2="' + mx + '" y2="' + (y + h - 4) + '" stroke="#999" stroke-width="0.3" opacity="0.4"/>');
                }
                for (var my = y + 4; my < y + h - 4; my += meshSpacing) {
                    parts.push('<line x1="' + (x + 8) + '" y1="' + my + '" x2="' + (x + w - 8) + '" y2="' + my + '" stroke="#999" stroke-width="0.3" opacity="0.4"/>');
                }
            }

            // Feeder liquid level
            if (compKey === 'feeder' && opts.feederHasSyrup) {
                var liquidH = h * 0.6;
                var liquidY = y + h - liquidH - 2;
                parts.push('<rect x="' + (x + 3) + '" y="' + liquidY + '" width="' + (w - 6) + '" height="' + liquidH + '" rx="1" fill="rgba(74,144,217,0.3)" stroke="none"/>');
            }

            // Inner cover notch
            if (compKey === 'inner_cover') {
                var notchW = 16;
                var notchH = 4;
                var notchX = x + (w / 2) - (notchW / 2);
                parts.push('<rect x="' + notchX + '" y="' + y + '" width="' + notchW + '" height="' + notchH + '" fill="' + cdef.strokeColor + '" opacity="0.5"/>');
            }

            // Labels on the right side
            if (showLabels) {
                var labelX = cx + BODY_WIDTH + 20;
                var labelY = y + (h / 2) + 4;
                var labelText = cdef.label;
                var wt = parseFloat(weights[cdef.dbWeightKey] || 0);
                if (wt > 0) labelText += ' (' + wt + ' lbs)';
                parts.push('<text x="' + labelX + '" y="' + labelY + '" font-size="11" fill="#555" font-family="Arial,sans-serif">' + escapeHtml(labelText) + '</text>');
            }

            y += h + COMPONENT_GAP;

            // Sensor indicator (drawn after the component at sensorPos)
            if (ri === sensorDrawnAt) {
                var sensorCx = cx + (BODY_WIDTH / 2);
                // Thermometer icon
                parts.push('<circle cx="' + sensorCx + '" cy="' + (y + 8) + '" r="7" fill="#d9534f" stroke="#c9302c" stroke-width="1"/>');
                parts.push('<rect x="' + (sensorCx - 2) + '" y="' + (y - 2) + '" width="4" height="10" rx="2" fill="#d9534f"/>');
                if (sensorValue) {
                    parts.push('<text x="' + (sensorCx + 14) + '" y="' + (y + 12) + '" font-size="11" fill="#d9534f" font-weight="bold" font-family="Arial,sans-serif">' + escapeHtml(sensorValue) + '</text>');
                }
                if (sensorLabel && showLabels) {
                    parts.push('<text x="' + (sensorCx + 14) + '" y="' + (y + 24) + '" font-size="9" fill="#999" font-family="Arial,sans-serif">' + escapeHtml(sensorLabel) + '</text>');
                }
                y += 20;
            }
        }

        // Stand/ground line
        parts.push('<line x1="' + (cx - 20) + '" y1="' + y + '" x2="' + (cx + BODY_WIDTH + 20) + '" y2="' + y + '" stroke="#8B7355" stroke-width="2"/>');

        parts.push('</svg>');

        if (typeof container === 'string') {
            container = document.querySelector(container);
        }
        if (container) {
            container.innerHTML = parts.join('');
        }
        return parts.join('');
    }

    function renderDashboard(containerId, config) {
        var stack = buildStackFromConfig(config);
        if (stack.length === 0) {
            var el = document.querySelector(containerId);
            if (el) {
                el.innerHTML = '<div style="text-align:center;color:#999;padding:20px;font-size:13px;">No hive components configured.<br><a href="/admin/hivebodyconfig.php">Configure Hive</a></div>';
            }
            return;
        }
        renderSVG(containerId, stack, {
            mode: 'dashboard',
            scale: 0.85,
            sensorPosition: config.sensorTempPosition,
            sensorValue: config.sensorValue || '',
            sensorLabel: config.sensorLabel || '',
            weights: config.weights || {},
            feederHasSyrup: config.feederHasSyrup || false,
            frameFeederPosition: config.frameFeederPosition,
            showLabels: false
        });
    }

    function renderEditor(containerId, stack, opts) {
        renderSVG(containerId, stack, {
            mode: 'editor',
            scale: 1,
            sensorPosition: opts.sensorPosition,
            sensorValue: '',
            sensorLabel: opts.sensorLabel || '',
            weights: opts.weights || {},
            feederHasSyrup: opts.feederHasSyrup || false,
            frameFeederPosition: opts.frameFeederPosition,
            showLabels: true
        });
    }

    // Editor interactivity: manages the component stack with add/remove/reorder
    function EditorController(config) {
        this.stack = buildStackFromConfig(config);
        this.weights = $.extend({}, config.weights || {});
        this.fixedWeights = {
            HIVE_TOP_WEIGHT: config.fixedWeights ? config.fixedWeights.HIVE_TOP_WEIGHT : 0,
            HIVE_COMPUTER_WEIGHT: config.fixedWeights ? config.fixedWeights.HIVE_COMPUTER_WEIGHT : 0,
            HIVE_MISC_WEIGHT: config.fixedWeights ? config.fixedWeights.HIVE_MISC_WEIGHT : 0
        };
        this.sensorPosition = (config.sensorTempPosition != null) ? config.sensorTempPosition : -1;
        this.sensorLabel = config.sensorLabel || 'Hive Temp';
        this.feederHasSyrup = config.feederHasSyrup || false;
        this.frameFeederPosition = (config.frameFeederPosition != null) ? config.frameFeederPosition : -1;
        this.frameFeederLabel = config.frameFeederLabel || 'Frame Feeder';
        this.diagramContainer = config.diagramContainer || '#hive-stack-diagram';
        this.listContainer = config.listContainer || '#hive-stack-list';
        this.tareContainer = config.tareContainer || '#tare-weight';
        this.onChangeCallback = config.onChange || null;
        this.placementMode = 'idle';
        this._dirty = false;
    }

    EditorController.prototype.addComponent = function (type) {
        if (!COMPONENT_DEFS[type]) return;
        var isBottom = (type === 'solid_bottom' || type === 'screened_bottom');
        var isTop = (type === 'tele_cover' || type === 'migratory_cover' || type === 'inner_cover');

        if (isBottom) {
            // Insert at position 0 (bottom of stack)
            this.stack.splice(0, 0, type);
        } else if (isTop) {
            this.stack.push(type);
        } else {
            // Insert above bottom board(s) but below covers
            var insertIdx = this.stack.length;
            for (var i = this.stack.length - 1; i >= 0; i--) {
                var k = this.stack[i];
                if (k === 'tele_cover' || k === 'migratory_cover' || k === 'inner_cover') {
                    insertIdx = i;
                }
            }
            this.stack.splice(insertIdx, 0, type);
        }
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.removeComponent = function (index) {
        if (index < 0 || index >= this.stack.length) return;
        this.stack.splice(index, 1);
        if (this.sensorPosition >= 0) {
            if (index <= this.sensorPosition) {
                this.sensorPosition--;
            }
            if (this.sensorPosition < 0 || this.sensorPosition >= this.stack.length - 1) {
                this.sensorPosition = -1;
            }
        }
        if (this.frameFeederPosition === index) {
            this.frameFeederPosition = -1;
        } else if (this.frameFeederPosition > index) {
            this.frameFeederPosition--;
        }
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.moveComponent = function (fromIndex, toIndex) {
        if (fromIndex === toIndex) return;
        var item = this.stack.splice(fromIndex, 1)[0];
        this.stack.splice(toIndex, 0, item);
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.setSensorPosition = function (position) {
        this.sensorPosition = (position === this.sensorPosition) ? -1 : position;
        this.placementMode = 'idle';
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.setFrameFeederPosition = function (position) {
        var compKey = this.stack[position];
        if (compKey !== 'deep' && compKey !== 'medium' && compKey !== 'shallow') return;
        this.frameFeederPosition = (position === this.frameFeederPosition) ? -1 : position;
        this.placementMode = 'idle';
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.enterPlacementMode = function (mode) {
        if (this.placementMode === mode) {
            this.placementMode = 'idle';
        } else {
            if (mode === 'sensor' && this.stack.length < 2) return;
            if (mode === 'feeder') {
                var hasBody = false;
                for (var i = 0; i < this.stack.length; i++) {
                    var k = this.stack[i];
                    if (k === 'deep' || k === 'medium' || k === 'shallow') { hasBody = true; break; }
                }
                if (!hasBody) return;
            }
            this.placementMode = mode;
        }
        this.refresh();
    };

    EditorController.prototype.cancelPlacement = function () {
        if (this.placementMode !== 'idle') {
            this.placementMode = 'idle';
            this.refresh();
        }
    };

    EditorController.prototype.getPlacementDescription = function (type) {
        if (type === 'sensor' && this.sensorPosition >= 0 && this.sensorPosition < this.stack.length - 1) {
            var below = COMPONENT_DEFS[this.stack[this.sensorPosition]].label;
            var above = COMPONENT_DEFS[this.stack[this.sensorPosition + 1]].label;
            return 'Between ' + above + ' and ' + below;
        }
        if (type === 'feeder' && this.frameFeederPosition >= 0 && this.frameFeederPosition < this.stack.length) {
            return 'In ' + COMPONENT_DEFS[this.stack[this.frameFeederPosition]].label;
        }
        return '';
    };

    EditorController.prototype.setFeederSyrup = function (hasSyrup) {
        this.feederHasSyrup = !!hasSyrup;
        this._dirty = true;
        this.refresh();
    };

    EditorController.prototype.updateWeight = function (dbKey, value) {
        this.weights[dbKey] = parseFloat(value) || 0;
        this._dirty = true;
        this.refreshTare();
    };

    EditorController.prototype.updateFixedWeight = function (key, value) {
        this.fixedWeights[key] = parseFloat(value) || 0;
        this._dirty = true;
        this.refreshTare();
    };

    EditorController.prototype.getTareWeight = function () {
        return computeTareWeight(this.stack, this.weights, this.fixedWeights);
    };

    EditorController.prototype.getQuantities = function () {
        var qtys = {};
        for (var key in COMPONENT_DEFS) {
            qtys[COMPONENT_DEFS[key].dbNumKey] = 0;
        }
        for (var i = 0; i < this.stack.length; i++) {
            var def = COMPONENT_DEFS[this.stack[i]];
            qtys[def.dbNumKey] = (qtys[def.dbNumKey] || 0) + 1;
        }
        return qtys;
    };

    EditorController.prototype.getStackOrder = function () {
        return this.stack.join(',');
    };

    EditorController.prototype.refresh = function () {
        this.renderDiagram();
        this.renderList();
        this.initSortable();
        this.initKeyboard();
        this.refreshTare();
        if (this.onChangeCallback) {
            this.onChangeCallback(this);
        }
    };

    EditorController.prototype.renderDiagram = function () {
        renderEditor(this.diagramContainer, this.stack, {
            sensorPosition: this.sensorPosition,
            sensorLabel: this.sensorLabel,
            weights: this.weights,
            feederHasSyrup: this.feederHasSyrup,
            frameFeederPosition: this.frameFeederPosition,
            frameFeederLabel: this.frameFeederLabel
        });
    };

    EditorController.prototype.renderList = function () {
        var $list = $(this.listContainer);
        if (!$list.length) return;
        var self = this;
        var items = [];
        var mode = this.placementMode;

        var $panel = $list.closest('.stack-panel');
        $panel.removeClass('placing-sensor placing-feeder');
        if (mode === 'sensor') $panel.addClass('placing-sensor');
        if (mode === 'feeder') $panel.addClass('placing-feeder');

        if (mode === 'sensor') {
            items.push('<li class="placement-instruction"><i class="fa fa-info-circle"></i> Click between components to place the temperature sensor. Press Escape to cancel.</li>');
        } else if (mode === 'feeder') {
            items.push('<li class="placement-instruction"><i class="fa fa-info-circle"></i> Click a highlighted body to place the frame feeder. Press Escape to cancel.</li>');
        }

        for (var i = this.stack.length - 1; i >= 0; i--) {
            var compKey = this.stack[i];
            var cdef = COMPONENT_DEFS[compKey];
            var isBody = (compKey === 'deep' || compKey === 'medium' || compKey === 'shallow');

            var liClasses = 'stack-item';
            if (mode === 'feeder' && isBody) {
                liClasses += ' feeder-eligible';
                if (this.frameFeederPosition === i) liClasses += ' feeder-current';
            }

            var feederBadge = '';
            if (mode === 'idle' && this.frameFeederPosition === i) {
                feederBadge = ' <span class="feeder-indicator-badge"><i class="fa fa-tint"></i> ' + escapeHtml(this.frameFeederLabel) + '</span>';
            }

            var dragHandle = (mode === 'idle')
                ? '<span class="drag-handle" style="cursor:grab;padding:4px 8px;margin-right:4px"><i class="fa fa-arrows-v" style="color:#ccc"></i></span>'
                : '';
            var removeBtn = (mode === 'idle')
                ? '<button type="button" class="btn btn-xs btn-danger remove-comp" data-idx="' + i + '">&times;</button>'
                : '';

            items.push(
                '<li data-stack-index="' + i + '" class="' + liClasses + '">' +
                '<div class="palette-card" style="border-left:4px solid ' + cdef.color + '">' +
                dragHandle +
                '<span class="card-info"><strong>' + escapeHtml(cdef.label) + '</strong>' + feederBadge + '</span>' +
                removeBtn +
                '</div>' +
                '</li>'
            );

            if (i > 0) {
                if (mode === 'sensor') {
                    var isCurrent = (this.sensorPosition === i - 1);
                    var dropClass = isCurrent ? 'sensor-drop-target current-position' : 'sensor-drop-target';
                    var dropText = isCurrent ? '<i class="fa fa-bullseye"></i> Current sensor position' : 'Place sensor here';
                    items.push('<li class="' + dropClass + '" data-sensor-pos="' + (i - 1) + '">' + dropText + '</li>');
                } else if (mode === 'idle' && this.sensorPosition === i - 1) {
                    items.push('<li class="sensor-indicator-badge"><i class="fa fa-bullseye"></i> ' + escapeHtml(this.sensorLabel) + '</li>');
                }
            }
        }

        $list.html(items.join(''));

        if (mode === 'idle') {
            $list.find('.remove-comp').off('click').on('click', function (e) {
                e.stopPropagation();
                self.removeComponent(parseInt($(this).data('idx'), 10));
            });
        } else if (mode === 'sensor') {
            $list.find('.sensor-drop-target').off('click').on('click', function () {
                self.setSensorPosition(parseInt($(this).data('sensor-pos'), 10));
            });
        } else if (mode === 'feeder') {
            $list.find('.feeder-eligible').off('click').on('click', function () {
                self.setFrameFeederPosition(parseInt($(this).data('stack-index'), 10));
            });
        }
    };

    EditorController.prototype.refreshTare = function () {
        var $tare = $(this.tareContainer);
        if ($tare.length) {
            $tare.text(this.getTareWeight().toFixed(2));
        }
    };

    EditorController.prototype.initKeyboard = function () {
        var self = this;
        $(document).off('keydown.hivePlacement').on('keydown.hivePlacement', function (e) {
            if (e.key === 'Escape' && self.placementMode !== 'idle') {
                self.cancelPlacement();
            }
        });
    };

    EditorController.prototype.initSortable = function () {
        var self = this;
        var $list = $(this.listContainer);
        if (!$list.length || !$.fn.sortable) return;

        if ($list.data('ui-sortable')) {
            $list.sortable('destroy');
        }

        if (this.placementMode !== 'idle') return;

        $list.sortable({
            items: 'li[data-stack-index]',
            axis: 'y',
            handle: '.drag-handle',
            tolerance: 'pointer',
            placeholder: 'ui-sortable-placeholder',
            update: function () {
                var newOrder = [];
                $list.find('li[data-stack-index]').each(function () {
                    newOrder.push(parseInt($(this).data('stack-index'), 10));
                });
                var oldToNew = {};
                var newStack = [];
                for (var i = newOrder.length - 1; i >= 0; i--) {
                    oldToNew[newOrder[i]] = newOrder.length - 1 - i;
                    newStack.push(self.stack[newOrder[i]]);
                }
                self.stack = newStack;
                if (self.frameFeederPosition >= 0 && oldToNew[self.frameFeederPosition] !== undefined) {
                    self.frameFeederPosition = oldToNew[self.frameFeederPosition];
                }
                self._dirty = true;
                self.renderDiagram();
                self.refreshTare();
                if (self.onChangeCallback) {
                    self.onChangeCallback(self);
                }
            }
        });
    };

    return {
        COMPONENT_DEFS: COMPONENT_DEFS,
        buildStackFromConfig: buildStackFromConfig,
        computeTareWeight: computeTareWeight,
        renderSVG: renderSVG,
        renderDashboard: renderDashboard,
        renderEditor: renderEditor,
        EditorController: EditorController
    };
})();
