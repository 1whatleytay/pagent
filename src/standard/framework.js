class $Component {
    $uuid = null

    $template() {
        return ``
    }

    $reload() {
        document.getElementById(this.$uuid).innerHTML = this.$template()
    }

    $render() {
        this.$uuid = $nextId()
        return `<div id="${this.$uuid}">${this.$template()}</div>`
    }
}

// function $insert(param) {
//     if (param instanceof String) {
//         return param.replace('<', '&lt;').replace('>', '$gt;')
//     } else if (Array.isArray(param)) {
//         return param.join('')
//     } else if (parma instanceof $Component) {
//         return $insert(param.$render())
//     }
// }

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
    document.getElementById('app').innerHTML = $route(window.location.href).$build().$render()
}

window.onload = $load
