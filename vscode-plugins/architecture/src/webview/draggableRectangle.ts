/**
 * @module
 * View component reusable by all views that are based on a draggable
 * rectangle, for instance, actors and buses.
 */

import { TrackedValue } from '../shared/architecture.js'

import { ChangeSet, TrackedIndex } from './changeSet.js'
import * as D from './dragHandlers.js'
import { XYRange } from './geometry.js'
import * as svg from './svg.js'
import { SystemServices } from './systemServices.js'

type TrackedIds = XYRange<TrackedIndex>

type TrackedValues = XYRange<TrackedValue<number>>

export class DraggableRectangle {

    readonly contentObject: SVGForeignObjectElement
    readonly rect: SVGRectElement
    readonly #sys: SystemServices
    readonly svgContainer: SVGSVGElement
    readonly trackedIds: TrackedIds

    constructor(
        sys: SystemServices,
        parentSVG: SVGSVGElement,
        trackedValues: TrackedValues,
    ) {
        this.#sys = sys
        this.trackedIds = {
            left: trackedValues.left.trackId,
            top: trackedValues.top.trackId,
            width: trackedValues.width.trackId,
            height: trackedValues.height.trackId,
        }

        // Offset for content
        const offset = { x: 20, y: 20 }

        // Create content
        const svgContainer = document.createElementNS(svg.ns, 'svg') as SVGSVGElement
        svg.setUserUnits(svgContainer.x, trackedValues.left.value)
        svg.setUserUnits(svgContainer.y, trackedValues.top.value)
        svg.setUserUnits(svgContainer.width, trackedValues.width.value)
        svg.setUserUnits(svgContainer.height, trackedValues.height.value)
        this.svgContainer = svgContainer
        sys.whenIntTrackChanged(this.trackedIds.left, (newValue) => {
            svgContainer.x.baseVal.value = newValue
        })
        sys.whenIntTrackChanged(this.trackedIds.top, (newValue) => {
            svgContainer.y.baseVal.value = newValue
        })
        sys.whenIntTrackChanged(this.trackedIds.width, (newValue) => {
            svgContainer.width.baseVal.value = newValue
        })
        sys.whenIntTrackChanged(this.trackedIds.height, (newValue) => {
            svgContainer.height.baseVal.value = newValue
        })

        this.rect = document.createElementNS(svg.ns, 'rect') as SVGRectElement
        this.rect.classList.add('enclave')
        svg.setPercentageUnits(this.rect.x, 0)
        svg.setPercentageUnits(this.rect.y, 0)
        svg.setPercentageUnits(this.rect.width, 100)
        svg.setPercentageUnits(this.rect.height, 100)

        D.addSVGDragHandlers(parentSVG, svgContainer, (e) => this.drag(e))

        this.contentObject = document.createElementNS(svg.ns, 'foreignObject') as SVGForeignObjectElement
        this.contentObject.x.baseVal.value = offset.x
        this.contentObject.y.baseVal.value = offset.y
        svg.setPercentageUnits(this.contentObject.width, 100)
        svg.setPercentageUnits(this.contentObject.height, 100)

        svgContainer.appendChild(this.rect)
        svgContainer.appendChild(this.contentObject)
        parentSVG.appendChild(svgContainer)
    }

    drag(evt: D.SVGDragEvent): void {
        const sys = this.#sys
        let newLeft = evt.left
        let newTop = evt.top

        // Adjust to not overlap
        newLeft = sys.adjustX(this, { top: newTop, height: this.height }, this.width, this.left, newLeft)
        newTop = sys.adjustY(this, { left: newLeft, width: this.width }, this.height, this.top, newTop)

        // Get new coordinates
        const r = {
            left: newLeft,
            top: newTop,
            right: newLeft + this.width,
            bottom: newTop + this.height,
        }

        if (!sys.overlaps(this, r)) {
            const changes = new ChangeSet()
            if (this.left !== newLeft) {
                changes.replace(this.trackedIds.left, newLeft)
            }
            if (this.top !== newTop) {
                changes.replace(this.trackedIds.top, newTop)
            }
            sys.sendUpdateDoc(changes)
        }
    }

    /** Remove all components from SVG */
    dispose(): void {
        this.svgContainer.remove()
    }

    get left(): number { return this.svgContainer.x.baseVal.value }
    get top(): number { return this.svgContainer.y.baseVal.value }
    get width(): number { return this.svgContainer.width.baseVal.value }
    get height(): number { return this.svgContainer.height.baseVal.value }
    get right(): number { return this.left + this.width }
    get bottom(): number { return this.top + this.height }
}
