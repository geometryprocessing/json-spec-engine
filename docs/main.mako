## _*_ coding: utf-8 _*_
<!DOCTYPE HTML>

<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">
<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/popper.js@1.12.9/dist/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>

<%def name="render_pointer(pointer_name)">
	${make_rules(rs = ijson.rules_from_pointer(pointer_name))}
</%def>

<%def name="make_rules(rs)">
		<% ## get a unique id
		attributes['count'] = attributes['count'] + 1
		local_count = attributes['count']
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
			r['doc'] = 'FIXME: Missing documentation in the specification' 
		r['pointer_short'] = '/' + r['pointer'].rsplit('/', 1)[-1] or s
		if r['pointer_short'] == '/*':
			tmp = r['pointer'].replace('/*','')
			r['pointer_short'] = ('/' + tmp.rsplit('/', 1)[-1] or s) + '/*'
		r['pointer_short'] = r['pointer']
		%>
		% if r['type'] == "object": ## Object
		<div id="accordion${local_count}">
  			<div class="card">
    			<div class="card-header" id="heading${local_count}">
      				<h5 class="mb-0">
						<button class="btn btn-link" data-toggle="collapse" data-target="#collapse${local_count}" aria-expanded="true" aria-controls="collapse${local_count}">
						<h4>${r['pointer_short']}</h4>
						</button>
						Object
						${"(Type: " + r["type_name"] + ")" if "type_name" in r else ""}
						% if "default" in r:
						[${r["default"]}]
						% endif
      				</h5>
					${r['doc']}
    			</div>
    			<div id="collapse${local_count}" class="collapse hide" aria-labelledby="heading${local_count}" data-parent="#accordion${local_count}">
					
					% if 'required' in r:
					<div class="container">
						<div class="row">
							<div class="col-1">
							<p class=".text-success">Required</p>
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
							<div class="col-1">
							<p class=".text-success">Optional</p>
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
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							Float
							% if "min" in r or "max" in r:
							(${r["min"] if "min" in r else "-inf"}, ${r["max"] if "max" in r else "inf"}) 
							% endif
							% if "default" in r:
							[${r["default"]}]
							% endif
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
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							Integer
							% if "min" in r or "max" in r:
							(${r["min"] if "min" in r else "-inf"}, ${r["max"] if "max" in r else "inf"}) 
							% endif
							% if "default" in r:
							[${r["default"]}]
							% endif
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
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							File 
							% if "extensions" in r:
							(${r["extensions"]}) 
							% endif 
							% if "default" in r:
							[${r["default"]}]
							% endif
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
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							Folder 
							% if "default" in r:
							[${r["default"]}]
							% endif
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
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							Boolean 
							% if "default" in r:
							[${r["default"]}]
							% endif
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
		% if r['type'] == "string": ## Bool
		<div class="card">
			<div class="card-header">
				<div class="container">
				<h5>
					<div class="row align-items-start">
						<div class="auto">
							<p class="text-primary">${r['pointer_short']}</p>
						</div>
						<div class="col-sm">
							Boolean
							% if "default" in r:
							[${r["default"]}]
							% endif
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
	<body>
	${render_pointer(pointer_name='/')}
	</body>
</html>