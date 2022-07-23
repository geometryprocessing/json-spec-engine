## _*_ coding: utf-8 _*_
<!DOCTYPE HTML>

<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.2.0/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-gH2yIJqKdNHPEq0n4Mqa/HGKIhSkIHeL5AyhkYV8i59U5AR6csBvApHHNl/vI1Bx" crossorigin="anonymous">
<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/popper.js@1.12.9/dist/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>

<style>
.row > * {
	width: auto !important;
}
body.dark {
	--bs-body-bg: var(--bs-dark);
	--bs-body-color: var(--bs-light);
}
body.dark .card {
	--bs-card-bg: var(--bs-gray-900);
	--bs-card-border-color: var(--bs-gray-700);
	--bs-card-cap-bg: var(--bs-gray-800);
}
body.dark .card-header {
	--bs-card-header-bg: var(--bs-dark);
	--bs-card-header-color: var(--bs-light);
}
body.dark button.btn {
	--bs-btn-color: var(--bs-light);
}

body.dark button.btn.btn-link {
	--bs-btn-color: var(--bs-link-color);
}

header {
	display: flex;
	justify-content: space-between;
	align-items: center;
}
}

#darkmode, #lightmode {
	float: right;
}
body.dark #lightmode {
	display: none;
}
body:not(.dark) #darkmode {
	display: none;
}

span.todo {
	color: var(--bs-red);
}

p.text-primary {
	margin-bottom: 0;
}
</style>

<script>
function initDarkMode() {
	const prefersDarkScheme = window.matchMedia("(prefers-color-scheme: dark)");
	if (prefersDarkScheme.matches) {
		document.body.classList.add("dark");
	} else {
		document.body.classList.remove("dark");
	}
}

function toggleDarkMode() {
  var element = document.body;
  element.classList.toggle("dark");
}
</script>

<%def name="clip_button(clip_text)">
	<button class="btn" onclick="navigator.clipboard.writeText('${clip_text}');">
	<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-clipboard" viewBox="0 0 16 16">
	<path d="M4 1.5H3a2 2 0 0 0-2 2V14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V3.5a2 2 0 0 0-2-2h-1v1h1a1 1 0 0 1 1 1V14a1 1 0 0 1-1 1H3a1 1 0 0 1-1-1V3.5a1 1 0 0 1 1-1h1v-1z"/>
	<path d="M9.5 1a.5.5 0 0 1 .5.5v1a.5.5 0 0 1-.5.5h-3a.5.5 0 0 1-.5-.5v-1a.5.5 0 0 1 .5-.5h3zm-3-1A1.5 1.5 0 0 0 5 1.5v1A1.5 1.5 0 0 0 6.5 4h3A1.5 1.5 0 0 0 11 2.5v-1A1.5 1.5 0 0 0 9.5 0h-3z"/>
	</svg>
	</button>
</%def>

<%def name="render_pointer(pointer_name)">
	${make_rules(rs = ijson.rules_from_pointer(pointer_name))}
</%def>

<%def name="make_rules(rs)">
		<% ## get a unique id
		attributes['count'] = attributes['count'] + 1
		local_count = attributes['count']
		%>
		<%
		if len(rs) > 0:
			rs[0]['pointer_last'] = rs[0]['pointer'].rsplit('/', 1)[-1]
			if rs[0]['pointer_last'] == '*':
				tmp = rs[0]['pointer'].replace('/*','')
				rs[0]['pointer_last'] = tmp.rsplit('/', 1)[-1]
		%>

		% if len(rs) == 1: ## single rules for a pointer use a cell
			${make_rule(r=rs[0])}
		% endif
		% if len(rs) > 1: ## multiple rules use a collapsible block
		<div id="accordion${local_count}">
  			<div class="card">
    			<div class="card-header" id="heading${local_count}">
      				<h5 class="mb-0">
						<button class="btn btn-link" data-toggle="collapse" data-target="#collapse${local_count}" aria-expanded="true" aria-controls="collapse${local_count}">
						<h4>${rs[0]['pointer']}</h4>
						</button>
						Multiple Types
      				</h5>
					## ${rs[0].get('doc', 'TODO').replace("TODO", '<span class="todo">TODO</span>')}
    			</div>
    			<div id="collapse${local_count}" class="collapse hide" aria-labelledby="heading${local_count}" data-parent="#accordion${local_count}">
					% for rule in rs:
					${make_rule(r=rule)}
					% endfor
    			</div>
  			</div>
		</div>
		% endif
</%def>

<%def name="make_rule(r)">
		<%
		attributes['count'] = attributes['count'] + 1
		local_count = attributes['count']
		if not 'doc' in r:
			r['doc'] = '<span class="todo">FIXME:</span> Missing documentation in the specification'
		if r["doc"] == "TODO":
			r["doc"] = '<span class="todo">TODO</span>'
		r['pointer_last'] = r['pointer'].rsplit('/', 1)[-1]
		if r['pointer_last'] == '*':
			tmp = r['pointer'].replace('/*','')
			r['pointer_last'] = tmp.rsplit('/', 1)[-1]
		r['pointer_short'] = r['pointer']
		%>
		% if r['type'] == "object": ## Object
		<div id="accordion${local_count}">
  			<div class="card">
    			<div class="card-header" id="heading${local_count}">
					<div class="container">
					<h5>
						<div class="row align-items-center">
							<div class="auto">
								<button class="btn btn-link" data-toggle="collapse" data-target="#collapse${local_count}" aria-expanded="true" aria-controls="collapse${local_count}">
								<h4>${r['pointer_short']}</h4>
								</button>
							</div>
							<div class="col-sm">
								<code>Object</code>
								${"(Type: " + r["type_name"] + ")" if "type_name" in r else ""}
								% if "default" in r:
								= ${r["default"]}
								% endif
							</div>
							<div class="auto">
							${clip_button(clip_text=r['pointer_last'])}
							</div>
						</div>
					</h5>
					</div>
					${r['doc']}
    			</div>
    			<div id="collapse${local_count}" class="collapse hide" aria-labelledby="heading${local_count}" data-parent="#accordion${local_count}">

					% if 'required' in r:

					<div class="container">

						<div class="row">
						<h5 class=".text-success">Required</h5>
							<div class="col-1">
							</div>
							<div class="col-11">
								% for fname in r['required']:
								${render_pointer(pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname))}
								% endfor
						    </div>
  						</div>
					</div>
					% endif
					% if 'optional' in r:
					<div class="container">

						<div class="row">
						<h5>Optional</h5>
							<div class="col-1">

							</div>
							<div class="col-11">
								% for fname in r['optional']:
								${render_pointer(pointer_name = (r['pointer']+fname) if r['pointer'] == '/' else (r['pointer']+'/'+fname))}
								% endfor
						    </div>
  						</div>
					</div>
					% endif

    			</div>
  			</div>
		</div>
		% endif
		% if r['type'] == "list": ## List
		${render_pointer(pointer_name = (r['pointer']+'*') if r['pointer'] == '/' else (r['pointer']+'/'+'*'))}
		% endif
		% if r['type'] == "float": ## Float
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>Float</code>
							% if "min" in r or "max" in r:
							(${r.get("min", "-inf")}, ${r.get("max", "inf")})
							% endif
							% if "default" in r:
							= ${r["default"]}
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>
					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
		% if r['type'] == "int": ## Int
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>Integer</code>
							% if "min" in r or "max" in r:
							(${r.get("min", "-inf")}, ${r.get("max", "inf")})
							% endif
							% if "default" in r:
							= ${r["default"]}
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>
					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
		% if r['type'] == "file": ## File
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>File</code>
							% if "extensions" in r:
							(${r["extensions"]})
							% endif
							% if "default" in r:
							= "${r["default"]}"
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>

					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
		% if r['type'] == "folder": ## Folder
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>Folder</code>
							% if "default" in r:
							= "${r["default"]}"
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>
					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
		% if r['type'] == "bool": ## Bool
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>Boolean</code>
							% if "default" in r:
							= ${r["default"]}
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>
					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
		% if r['type'] == "string": ## String
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-center">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							<code>String</code>
							% if "options" in r:
							(${r["options"]})
							% endif
							% if "default" in r:
							= "${r["default"]}"
							% endif
						</div>
						<div class="auto">
						${clip_button(clip_text=r['pointer_last'])}
						</div>
					</div>
				</h5>
				</div>
  			</div>
  			<div class="card-body">
    			<p class="card-text">${r['doc']}</p>
			</div>
		</div>
		% endif
</%def>

<html>
	<body onload="initDarkMode()">

    <header>
		<h2>${title}</h2>
		<button onclick="toggleDarkMode()" class="btn" id="lightmode">
			<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="currentColor" class="bi bi-brightness-low" viewBox="0 0 24 24">
			<path d="M12 8a4 4 0 0 0-4 4 4 4 0 0 0 4 4 4 4 0 0 0 4-4 4 4 0 0 0-4-4m0 10a6 6 0 0 1-6-6 6 6 0 0 1 6-6 6 6 0 0 1 6 6 6 6 0 0 1-6 6m8-9.31V4h-4.69L12 .69 8.69 4H4v4.69L.69 12 4 15.31V20h4.69L12 23.31 15.31 20H20v-4.69L23.31 12 20 8.69Z"></path>
			</svg>
		</button>
		<button onclick="toggleDarkMode()" class="btn" id="darkmode">
			<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="currentColor" class="bi bi-brightness-low" viewBox="0 0 24 24">
				<path d="M12 18c-.89 0-1.74-.2-2.5-.55C11.56 16.5 13 14.42 13 12c0-2.42-1.44-4.5-3.5-5.45C10.26 6.2 11.11 6 12 6a6 6 0 0 1 6 6 6 6 0 0 1-6 6m8-9.31V4h-4.69L12 .69 8.69 4H4v4.69L.69 12 4 15.31V20h4.69L12 23.31 15.31 20H20v-4.69L23.31 12 20 8.69Z"></path>
			</svg>
		</button>
	</header>

	${render_pointer(pointer_name='/')}

	</body>
</html>