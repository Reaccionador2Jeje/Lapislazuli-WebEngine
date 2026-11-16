//! Minimal URL parser prototype; not WHATWG-compliant or a security boundary.

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ParsedUrl {
    pub scheme: String,
    pub authority: String,
    pub path_and_query: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum UrlError { MissingScheme, InvalidScheme, MissingAuthority }

pub fn parse_absolute(input: &str) -> Result<ParsedUrl, UrlError> {
    let (scheme, remainder) = input.split_once("://").ok_or(UrlError::MissingScheme)?;
    let mut chars = scheme.chars();
    let first = chars.next().ok_or(UrlError::InvalidScheme)?;
    if !first.is_ascii_alphabetic()
        || !chars.all(|c| c.is_ascii_alphanumeric() || matches!(c, '+' | '-' | '.'))
    {
        return Err(UrlError::InvalidScheme);
    }
    let boundary = remainder.find(['/', '?', '#']).unwrap_or(remainder.len());
    let authority = &remainder[..boundary];
    if authority.is_empty() { return Err(UrlError::MissingAuthority); }
    let path_and_query = if boundary == remainder.len() {
        "/".to_string()
    } else if matches!(remainder.as_bytes()[boundary], b'?' | b'#') {
        format!("/{}", &remainder[boundary..])
    } else {
        remainder[boundary..].to_string()
    };
    Ok(ParsedUrl {
        scheme: scheme.to_ascii_lowercase(),
        authority: authority.to_string(),
        path_and_query,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn parses_https_url() {
        let url = parse_absolute("https://example.com/docs?q=1").unwrap();
        assert_eq!(url.scheme, "https");
        assert_eq!(url.authority, "example.com");
        assert_eq!(url.path_and_query, "/docs?q=1");
    }
    #[test]
    fn rejects_missing_scheme() {
        assert_eq!(parse_absolute("example.com"), Err(UrlError::MissingScheme));
    }
    #[test]
    fn rejects_empty_authority() {
        assert_eq!(parse_absolute("https:///path"), Err(UrlError::MissingAuthority));
    }
}
