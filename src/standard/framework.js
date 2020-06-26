class $Component {
    $uuid = null

    $template() {
        return ``
    }

    $reload() {
        document.getElementById(this.$uuid).innerHTML = this.$template()
        return this
    }

    $render() {
        this.$uuid = $nextId()
        return `<div id="${this.$uuid}">${this.$template()}</div>`
    }

    $build() {
        return this
    }
}

function $alignStyle(value) {
    switch (value) {
        case 'center':
            return ''
        case 'left':
            return ''
        case 'right':
            return ''
        default:
            return 0
    }
}

function $clean(text) {
    return text
        .replace('&', '&amp;')
        .replace(';', '&semi;')
        .replace('<', '&lt;')
        .replace('>', '$gt;')
}

function $insert(param) {
    if (Array.isArray(param)) {
        return param.join('')
    } else if (param instanceof $Component) {
        return $insert(param.$render())
    } else {
        return param
    }
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
    document.getElementById('app').innerHTML = $route(window.location.pathname).$build().$render()
}

window.onload = $load
