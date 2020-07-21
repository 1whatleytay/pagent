class $Component {
    $uuid = null

    $template() {
        return ``
    }

    $reload() {
        console.log({ element: document.getElementById(this.$uuid).outerHTML, template: this.$template() })
        document.getElementById(this.$uuid).outerHTML = this.$template()
        return this
    }

    $render() {
        this.$uuid = $nextId()
        return this.$template()
    }

    $build() {
        return this
    }
}

function $directionStyle(value) {
    return `${value.top * 2}px ${value.right * 2}px ${value.bottom * 2}px ${value.left * 2}px`
}

const Align$center = 'center'
const Align$left = 'left'
const Align$right = 'right'

const ContentAlign$center = 'center'
const ContentAlign$top = 'top'
const ContentAlign$bottom = 'bottom'

function $alignStyle(value) {
    switch (value) {
        case Align$center:
            return 'margin-left: auto;margin-right: auto;'
        case Align$left:
            return 'margin-right: auto;'
        case Align$right:
            return 'margin-left: auto;'
        default:
            console.error(`Unknown align style value ${value}.`)
            return ''
    }
}

function $textAlignStyle(value) {
    switch (value) {
        case Align$center:
            return 'center'
        case Align$left:
            return 'left'
        case Align$right:
            return 'right'
        default:
            console.error(`Unknown text align style value ${value}.`)
            return ''
    }
}

function $contentAlignStyle(value) {
    switch (value) {
        case ContentAlign$center:
            return 'align-items: center;'
        case ContentAlign$top:
            return 'align-items: flex-start;'
        case ContentAlign$bottom:
            return 'align-items: flex-end;'
        default:
            console.error(`Unknown content align style value ${value}.`)
            return ''
    }
}

function $borderStyle(value) {
    let result = 'border-style: solid;'

    if (value.size) {
        result += `border-width: ${value.size * 2}px;`
    }

    if (value.color) {
        result += `border-color: #${value.color.hex};`
    }

    return result
}

function $clean(text) {
    if (text === null) {
        return ''
    }

    if (typeof text === 'number') {
        return text.toString()
    }

    return text
        .replace('&', '&amp;')
        .replace(';', '&semi;')
        .replace('<', '&lt;')
        .replace('>', '&gt;')
}

function $insert(param) {
    if (Array.isArray(param)) {
        return param.map(x => $insert(x)).join('')
    }

    if (param instanceof $Component) {
        return $insert(param.$render())
    }

    if (param === null) {
        return ''
    }

    return param
}

let $id = 0

function $nextId() {
    return $id++
}

let $events = [ ]

function $routeEvent(event) {
    let index = $events.length
    $events.push(event)
    return index
}

function $callEvent(event) {
    // todo: arguments stuff
    $events[event]()
}

function $load() {
    document.getElementById('app').innerHTML = window.$route(window.location.pathname).$build().$render()
}

Array.prototype.empty = function() {
    return this.length === 0
}

window.onload = $load
